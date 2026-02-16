# Conversation: 2026-02-16

| Field | Value |
|-------|-------|
| Date | 2026-02-16 |
| Session ID | `623ed273-2316-4fd8-a700-c10fd07213ff` |
| Model | claude-opus-4-6 |
| Tokens | ~44,862 input, ~3 output |
| Duration | 6s |

---

## User (03:32 UTC)

<teammate-message teammate_id="devops-engineer" color="green" summary="Tooling gap analysis: web vs UE agent workflows">
## Task 3: Tooling Gap Analysis — Web vs UE Agent Workflows

### The Fundamental Gap

The web agent workflow is a **tight, synchronous loop**: write code -> save -> browser renders -> agent reads DOM/screenshot -> iterate. Total cycle: ~2 seconds. The agent never leaves its process. The browser IS the runtime.

The UE agent workflow is a **fragmented, asynchronous pipeline**: write C++ -> compile (12-26s) -> kill stale processes -> launch editor (30-60s) -> wait for RC API (2-120s) -> send HTTP commands -> read state via HTTP -> take screenshot via macOS screencapture -> hope screenshot is readable. Total cycle: ~2-5 minutes, assuming nothing goes wrong. The agent operates through 4 layers of indirection (filesystem -> compiler -> editor process -> HTTP API -> screencapture).

This is not a matter of "better tooling will close the gap." The gap is architectural. UE was designed for humans staring at a screen. Web was designed for machines parsing text. The question is: what tooling investments bring UE closer to the web's feedback density without fighting the engine's design?

---

### Current State of PlayUnreal

**What exists (all committed, partially verified in Sprint 8 hotfix):**

| Component | LOC | Status |
|-----------|-----|--------|
| `client.py` | 708 | Verified: diagnose() and get_state() work against live editor |
| `run-playunreal.sh` | 230 | Verified: launches editor, polls RC API, runs Python scripts |
| `verify_visuals.py` | 284 | Partially verified (ran once in hotfix) |
| `diagnose.py` | 408 | Verified: all 7 phases produce useful output |
| `acceptance_test.py` | 160 | UNVERIFIED: never run against live game |
| `qa_checklist.py` | ~200 | UNVERIFIED |
| `run-tests.sh` | 382 | Fully verified, 170 tests pass reliably |

**What works:**
- RC API connection and object discovery (diagnose.py confirmed live GameMode and FrogCharacter paths)
- GetGameStateJSON() returns valid JSON with all expected keys
- RequestHop accepts FVector parameter format and moves the frog
- macOS screencapture with window ID targeting produces PNGs
- Lock file prevents concurrent test runs (Section 19)

**What doesn't work or is fragile:**
- Object path discovery is brute-force (13 candidate paths for GameMode, 7 for Frog). If UE changes its naming convention, all paths break silently.
- Video recording: hard 3-second cap on macOS with `-l` flag. Cannot capture longer sequences.
- Screenshot timing: agent takes screenshot, but CANNOT READ the screenshot. Claude Code can read images, but the PlayUnreal scripts have no mechanism to feed screenshots back into the agent's reasoning loop.
- Editor startup: 30-120 seconds. No way to keep the editor running across multiple agent invocations.
- No hot reload: every C++ change requires full recompile + relaunch.

**The critical missing piece: screenshot-in-the-loop.**
PlayUnreal can TAKE screenshots. But the agent workflow doesn't CONSUME them. The screenshots sit in `Saved/Screenshots/` and nobody looks at them. In web dev, the browser rendering IS the feedback. In our UE workflow, we take a photo of the output and then... file it away. The loop is open.

---

### What Would Close the Gap

**Tier 1: Things that close the loop (highest impact)**

1. **Persistent editor with hot-reload workflow.** Instead of launch-test-kill per invocation, keep the editor running in `-game` mode between agent actions. UE 5.7's Live Coding (`Ctrl+Alt+F11` / console command `LiveCoding.Compile`) can recompile C++ while the game runs. The agent workflow becomes: edit C++ -> trigger Live Coding compile via RC API -> verify state via get_state() -> screenshot. This cuts the cycle from 2-5 minutes to ~15-30 seconds.
   - **Risk:** Live Coding has known limitations (can't add new UPROPERTYs, can't change class layout, can't add new classes). But for tuning, bug fixes, and behavioral changes, it works.
   - **Implementation:** Add a `live_compile()` method to client.py that sends `LiveCoding.Compile` console command via RC API's `/remote/object/call` on the engine CDO. Add a `--keep-alive` flag to run-playunreal.sh that skips the cleanup trap.

2. **Screenshot-as-feedback integration.** When an agent takes a screenshot via PlayUnreal, the script should output the absolute path in a structured format that Claude Code can auto-read. Better yet: the PlayUnreal Python scripts should be callable as a "skill" from Claude Code, and the screenshot path should be returned in a way that triggers Claude Code's image reading.
   - **Current gap:** `verify_visuals.py` takes 6+ screenshots but prints only text. The agent never sees what it captured.
   - **Proposal:** After each screenshot, print `[SCREENSHOT] /absolute/path/to/file.png` — then the agent's next step is to Read that file. This is a process change more than a code change.

3. **State-diff instead of state-snapshot.** `get_state()` returns a flat JSON blob. What the agent really needs is: "what changed since the last call?" A diff-aware client that tracks previous state and highlights deltas (score went from 0 to 10, frog moved from [6,0] to [6,1], lives dropped from 3 to 2) would make state changes immediately parseable without the agent having to compare JSON blobs mentally.

**Tier 2: Things that reduce friction (medium impact)**

4. **Deterministic object path resolution.** Replace the brute-force 13-candidate search with a single `GetWorld()->GetAuthGameMode()` exposed as a UFUNCTION that returns the object path string. Same for the Frog: `GetWorld()->GetFirstPlayerController()->GetPawn()`. One RC API call instead of 13 sequential HTTP PUTs with error handling.

5. **Build-and-verify in one command.** Merge `run-tests.sh` and `run-playunreal.sh` into a single workflow: `./Tools/PlayUnreal/build-test-verify.sh` that does: UBT build -> headless unit tests -> launch game -> visual verification -> screenshot -> report. The agent runs one command and gets a complete picture. Currently, the agent must sequence 3 separate commands and interpret 3 separate outputs.

6. **Console command passthrough.** The RC API supports calling any UFUNCTION. We should expose a generic `exec_console(cmd)` method on client.py that sends any UE console command (e.g., `stat fps`, `show collision`, `viewmode wireframe`). This gives agents debugging superpowers without adding new UFUNCTIONs for each diagnostic.

**Tier 3: Aspirational (long-term, high effort)**

7. **Editor MCP server.** Instead of HTTP polling, implement a Model Context Protocol server inside the editor that speaks MCP natively. This would let Claude Code treat the running UE editor as a tool provider, with functions like `call_ufunction`, `read_uproperty`, `screenshot`, `get_viewport_image` available as first-class tools. This is the "browser DevTools for UE" vision.

8. **Viewport pixel buffer via RC API.** Instead of macOS screencapture (which requires window visibility and has timing issues), capture the viewport framebuffer directly from the engine via a UFUNCTION that reads the backbuffer to a PNG. UE has `FScreenshotRequest` and `UGameViewportClient::OnScreenshotCaptured` for this. This removes the macOS dependency entirely and works in headless mode.

---

### Top 3 Concrete Proposals (Ranked by Impact/Effort)

**Proposal 1: Keep-alive editor + Live Coding integration**
- Add `--keep-alive` to run-playunreal.sh (skip cleanup on exit)
- Add `live_compile()` to client.py
- Agent workflow: edit -> compile in-place -> verify -> screenshot
- Estimated impact: cuts iteration cycle from 2-5 min to 15-30 sec
- Effort: ~1 sprint day

**Proposal 2: Screenshot-in-the-loop process**
- After every screenshot, print structured path for Claude Code to Read
- Modify agent profiles to include "after running PlayUnreal, always Read the screenshot files"
- Create a `quick_check.py` script: take screenshot, print path, print state diff — nothing else
- Estimated impact: closes the open feedback loop that has been the root cause for 7 sprints
- Effort: ~2 hours of script changes + agent profile updates

**Proposal 3: Deterministic object paths + state diff**
- Add `GetObjectPath()` UFUNCTION to GameMode that returns its own path
- Add state-diff tracking to client.py
- Eliminate 13-candidate brute force search
- Estimated impact: faster, more reliable RC API interactions; clearer agent reasoning about state changes
- Effort: ~1 sprint day

---

### The Honest Assessment

We will never achieve web-dev parity. The compilation step alone is an irreducible bottleneck. A Three.js game in index.html can be modified and refreshed in under a second because JavaScript is interpreted, the browser is the runtime, and the rendering pipeline is a single function call. UE C++ requires compilation, linking, editor initialization, world loading, and actor spawning — each step is necessary and none can be skipped.

But we can go from "the agent literally cannot see what it built" (current state) to "the agent sees a screenshot of its changes within 30 seconds of making them" (Proposals 1+2). That is a 10x improvement in feedback density. Not web-dev parity, but enough to prevent 7 more sprints of invisible VFX code.

The root cause of every visual verification failure in Sprints 2-8 is the same: the feedback loop is open. We write code, we test code, we never look at the result. Proposals 1 and 2 close the loop. Everything else is optimization.

</teammate-message>

## Assistant (03:32 UTC)

Excellent analysis from DevOps. That's 1 of 4. Let me note the key points and continue waiting for the remaining three.

**DevOps key insights captured:**
- Architectural gap: web is 2s feedback loop, UE is 2-5 min through 4 layers of indirection
- Compilation is an irreducible bottleneck -- web-dev parity is impossible
- But 10x improvement is achievable: Live Coding + screenshot-in-the-loop
- The root cause across 7 sprints: open feedback loop (write code, never look at result)
- Top proposals: keep-alive editor with Live Coding, screenshot-as-feedback, deterministic object paths

Still waiting for: **engine-architect**, **game-designer**, **qa-lead**.
