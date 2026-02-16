# Team Working Agreements

*This is a living document. Updated during retrospectives by the XP Coach.*
*Last updated: Sprint 8 Retrospective*

## Day 0 Agreements

These are our starting agreements. They WILL evolve through retrospectives.

### 1. Collaboration Model (Updated: Sprint 7 Retrospective)

- **Multiple perspectives on every task.** No agent works alone. Every task gets differing opinions at three stages: design debate before code, active navigation during implementation, and cross-domain review before commit. Solo-driver delivery (Sprints 2-6) optimized for speed but produced blind spots. We are reversing that tradeoff — the token cost of multiple opinions is worth it.
- **One writer per file at a time.** This is a file-safety rule, not a thinking rule. Multiple drivers are OK if they touch zero overlapping files. (Updated Sprint 1.)
- **Always spawn a challenger.** For every task, at least one agent from a *different* domain than the driver must be active and providing input. The challenger's job is to question assumptions, not rubber-stamp.
- **Driver rotation**: The XP Coach assigns the driver for each task based on domain expertise.
- **Any agent can call "Stop"** if they see a problem. The driver must pause and discuss.
- **Active navigation is mandatory, not aspirational.** While the driver implements, at least 1 navigator actively reviews via messages — questioning design choices, flagging edge cases, suggesting alternatives. For tasks >200 lines, the driver pauses after each logical subsystem and posts a summary (what was built, what tests cover it, what comes next). (Updated from Sprint 5 review-gate rule: navigation is now continuous, not checkpoint-only.)
- **Accept message lag as normal.** Multi-agent sessions have unreliable message ordering. When an agent reports stale state, verify before escalating. Do not send repeated "you are unblocked" messages — one clear message with verification steps is sufficient. (Added: Sprint 7 Retrospective — multiple agents operated on stale context throughout the sprint, wasting coordination tokens.)

### 2. TDD is Mandatory

- Write the failing test FIRST (Red)
- Implement the minimum code to pass (Green)
- Refactor while tests still pass (Refactor)
- No code merges without passing tests
- **Seam tests are mandatory for interacting systems.** For every pair of systems that interact (e.g., frog + moving platform, frog + submerging turtle, pause + hazard movement), write at least one test that exercises their boundary. Isolation tests verify each system alone; seam tests verify the handshake between them. (Added: Post-Sprint 2 Hotfix Retrospective — 33% of commits were fixes at system boundaries that isolation tests missed.)
- **Seam test coverage matrix.** Maintain a seam test matrix in `Docs/Testing/seam-matrix.md` listing all pairs of systems with runtime interactions. Each pair must have either a test case name or an explicit "deferred — low risk" acknowledgment. Review the matrix before marking a sprint complete and add tests for any new seams. (Added: Sprint 4 Retrospective — round restart bug was a missed seam between RoundComplete → OnSpawningComplete → Respawn.)
- **Floating-point boundary testing.** When writing tests for threshold/boundary conditions, compute exact decimal values and pick test inputs with clear margin from the boundary. Never rely on mental arithmetic for floating-point comparisons (e.g., 5.0/30.0 = 0.1667 is BELOW 0.167, not above). (Added: Sprint 6 Retrospective — timer warning seam test had a boundary math error.)
- **Tuning-resilient test design.** When writing tests for tunable values (difficulty curves, timing thresholds), read the actual parameter from the game object at test time instead of hardcoding expected values. Tests should verify the *formula* is correct, not specific magic numbers. (Added: Sprint 7 Retrospective — codifies QA Lead's Seam 16 refactor pattern.)

### 3. Communication Protocol

- **Design debate before implementation.** Before any code is written, at least 2 agents must propose approaches via mailbox. Proposals can be brief (3-5 sentences) but must present a distinct approach or tradeoff. The Team Lead synthesizes or picks the best. Disagreement is encouraged — if everyone agrees immediately, the challenger isn't doing their job. (Updated: replaces "post a 3-sentence plan and wait for acknowledgment" — that allowed solo thinking.)
- Use the shared task list for work coordination
- Use direct messages for design discussions and active navigation
- Disagreements are resolved by the domain expert (Engine Architect for C++, Game Designer for mechanics, etc.) but the dissenting opinion must be heard and acknowledged before resolution

### 4. Commit Standards

- Conventional Commits format: `feat:`, `fix:`, `refactor:`, `test:`, `docs:`, `chore:`
- Each commit is one logical change
- **Sprints must not be delivered as a single monolithic commit.** Each independently testable subsystem (new class + its tests, new tool/script, configuration changes) must be its own commit. A sprint delivering N new subsystems should have at minimum N commits. The only exception is tightly coupled changes where one subsystem cannot compile without the other. (Added: Sprint 5 Retrospective -- Sprints 4 and 5 were each delivered as single 2000+ line commits, eliminating rollback granularity and intermediate review.)
- Commit message explains WHY, not WHAT (the diff shows what)
- Never commit broken builds
- **Build verification is mandatory before every commit.** Run the UBT build for BOTH Game AND Editor targets and confirm `Result: Succeeded` before `git commit`. Never build just one target — the `-game` flag uses Editor binaries. (Updated Sprint 2: building only the Game target caused debugging confusion.)

### 5. Feature Workflow

1. XP Coach breaks down the feature into tasks with dependency graph
2. **Design debate** — At least 2 agents propose competing approaches. Team Lead synthesizes. (Updated: replaces "team discusses" — debate requires distinct proposals, not passive agreement.)
3. **Same agent writes test AND implementation** (prevents API mismatch — learned Sprint 1)
4. Domain expert drives implementation **with active navigator(s)** providing real-time input via messages
5. **Cross-domain review** — An agent from a *different* domain than the driver reviews before commit (e.g., QA Lead reviews Engine Architect's code; Engine Architect reviews Game Designer's tuning)
6. **Build verification** — UBT build must succeed (Game + Editor targets)
7. Tests pass → merge
8. **Play-test before marking gameplay tasks complete.** Run `play-game.sh` and execute the feature manually. One minute of play-testing catches bugs that 100 tests miss. (Updated: Sprint 4 Retrospective)
9. **QA Lead play-tests BEFORE the sprint commit is created**, not after. If QA finds defects, they are fixed and re-tested before committing. The commit message may include "QA: verified" only after explicit QA sign-off. If QA play-test is not possible (no display, no QA agent available), the commit message must note "QA: pending" and a follow-up play-test is a P0 for the next session. (Updated: Sprint 5 Retrospective -- QA play-test was skipped in Sprint 5, breaking the Definition of Done.)
10. XP Coach runs retrospective

### 5a. Definition of Done (Updated: Sprint 8 Retrospective)

A sprint is NOT done until ALL of the following are completed **in order**. Each step BLOCKS the next -- no skipping.

1. **Build succeeds** — Game + Editor targets both pass UBT build
2. **All automated tests pass** — `run-tests.sh --all` reports 0 failures
3. **Seam tests exist** for all new system interactions introduced this sprint
4. **PlayUnreal visual verification passes** — For any commit that modified visual output (VFX, HUD, materials, camera, lighting, ground), PlayUnreal or manual play-testing has produced screenshots saved to `Saved/Screenshots/` showing the effect is visible from the gameplay camera. If NO visual changes were made this sprint, this step is N/A. (Added: Sprint 8 Retrospective -- Section 9 was violated for 7 consecutive sprints because it was a suggestion, not a gate.)
5. **QA Lead signs off** — QA Lead has run the game (via PlayUnreal or manually) and confirmed: the game is playable, visual effects are visible, gameplay feels correct. QA cannot sign off until step 4 is complete.
6. **The project is left in a working state** for the next sprint

**Why the ordering matters (Sprint 8):** The flat unordered list allowed the team to skip visual verification (old step 4) while completing other steps. Seven sprints of "tests pass but game looks identical" proves that an unordered checklist is not enforced. Ordered gates ensure the most-skipped step (visual verification) cannot be bypassed.

### 6. Asset Pipeline

- All generated assets go through the validation pipeline before import
- Art Director approves visual assets
- Sound Engineer approves audio assets
- DevOps Engineer verifies import automation works

### 7. Retrospectives

- Mandatory after each completed feature or sprint deliverable (even if multiple features ship together). If a sprint delivers via a single commit, the retrospective happens immediately after that commit lands. (Updated: Sprint 4 Retrospective)
- XP Coach facilitates using the retrospective skill
- Produces concrete changes to THIS document and/or agent profiles
- Changes are committed as part of the retrospective

### 8. PlayUnreal Test Harness (Added: Stakeholder Review)

- Agents must be able to launch the game and interact with it programmatically
- Build a PlayUnreal automation tool (similar to PlayGodot) that can:
  - Launch the editor with a test map
  - Send input commands (hop, pause, etc.) via Remote Control API or Automation Driver
  - Capture game state (frog position, score, lives, game state)
  - Take screenshots for visual verification
- QA Lead and Game Designer use PlayUnreal to validate gameplay feel
- DevOps Engineer owns the PlayUnreal infrastructure

### 9. Visual Verification Is Mandatory, Not Aspirational (Updated: Sprint 8 Retrospective)

**This section has been violated in every sprint since it was created (Sprints 2-8). The problem is not the rule -- it is enforcement. This version replaces checklists with a hard gate.**

**The rule:** Any commit that modifies visual output MUST be accompanied by a screenshot or PlayUnreal log proving the change is visible in the running game. "Visual output" means: VFX effects, HUD elements, materials, camera, lighting, ground appearance, score pops, death flash, wave announcements, or any other change intended to be seen by the player.

**How to verify:**
1. **Preferred:** Run `Tools/PlayUnreal/run-playunreal.sh <script>` where `<script>` triggers the visual element and takes a screenshot. Save to `Saved/Screenshots/`.
2. **Fallback (if PlayUnreal is not operational):** Launch the game manually per Section 14 commands. Take a macOS screenshot (Cmd+Shift+4). Save to `Saved/Screenshots/`.
3. **Last resort:** If neither method works, the commit message MUST include `visual verification: SKIPPED -- [specific reason]`. This marks the commit as a draft that BLOCKS sprint completion (see Section 5a step 4).

**XP Coach enforcement:** The XP Coach MUST verify that screenshot evidence exists before approving any visual commit. If no evidence exists and no "SKIPPED" note is in the commit message, the XP Coach rejects the commit. This is a hard gate.

**What to verify (unchanged from previous version):**
- Is the effect visible from the gameplay camera at Z=2200?
- Is it at the correct position relative to the action that triggered it?
- Is it large enough and long enough to be noticed by a player?
- Does it look correct at the actual camera distance?

**Why (Sprint 2):** Sprint 2 built 12 tasks before launching. Three critical visual bugs invisible to unit tests.
**Why (Sprint 7):** Score pops hardcoded to top-left. Death VFX too small. Both passed all tests but invisible to players.
**Why (Sprint 8):** 888 lines of VFX/HUD code, 170 passing tests, zero visual verification performed. PlayUnreal was built in the same sprint and never used. Stakeholder confirmed: game looks identical to pre-Sprint 8. This section was violated for the 7th consecutive sprint.

### 10. Scene Requirements for New Maps (Added: Sprint 2 Retrospective)

When creating a new map or empty scene via code, always ensure:
- A directional light exists (sun)
- Ambient lighting exists (sky light or fill light)
- Camera view target is explicitly set
- HUD is wired to game state (polling or delegates)
- Ground/floor geometry is present and lit

**Why:** An empty .umap has NO lights, NO sky, NO atmosphere. C++ auto-spawned meshes are invisible without lighting.

### 11. UE5 Material Coloring (Added: Post-Sprint 2 Hotfix)

**Never call `SetVectorParameterValue("BaseColor", ...)` on engine primitive meshes.** UE5's `BasicShapeMaterial` has no exposed "BaseColor" parameter — the call silently does nothing and meshes render gray.

To tint engine primitives (Cube, Sphere, Cylinder):
1. Use `GetOrCreateFlatColorMaterial()` from `Core/FlatColorMaterial.h` — creates a runtime UMaterial with a "Color" VectorParameter wired to BaseColor
2. Call `Component->SetMaterial(0, FlatColor)` FIRST
3. Then `CreateAndSetMaterialInstanceDynamic(0)` to get a DynMat
4. Then `DynMat->SetVectorParameterValue(TEXT("Color"), DesiredColor)`

**Note:** `FlatColorMaterial.h` uses `WITH_EDITORONLY_DATA` — it returns nullptr in packaged builds. For shipping, create a persistent `.uasset` material.

**Why:** Sprint 2 shipped with all meshes gray. All actors called SetVectorParameterValue but nothing was visible because the base material had no parameter to set.

### 12. Overlap Detection Timing (Added: Post-Sprint 2 Hotfix)

**Never rely on deferred overlap events for same-frame logic.** `SetActorLocation()` without sweep defers overlap events to the next physics tick. Frame-counting grace periods (e.g., `bJustLanded`) are unreliable across different tick rates and physics sync timings.

For immediate platform/collision detection after teleporting an actor:
- Use **direct position-based checks** via `TActorIterator<T>` and distance comparisons
- Compare actor positions and known extents — no physics engine dependency
- This is fast for small actor counts (<50 hazards) and avoids physics sync race conditions
- Reserve `OverlapMultiByObjectType` for queries where physics bodies are guaranteed to be synced (e.g., during physics tick callbacks)

**Why:** The frog always died on river logs because `FinishHop()` queried overlaps at the landing position, but physics bodies hadn't synced yet from the current frame's `SetActorLocation` calls.

### 13. Delegate Wiring Verification (Added: Post-Sprint 2 Hotfix)

**Defining a delegate method is not the same as binding it.** Every delegate (`OnHopCompleted`, `OnFrogDied`, etc.) must have an explicit `AddDynamic`/`AddLambda`/`AddUObject` call somewhere in the codebase. Having a handler method exist does nothing if nobody calls `AddDynamic`.

Verification rule: For every `DECLARE_DYNAMIC_MULTICAST_DELEGATE`, grep for at least one `AddDynamic` binding. If zero bindings exist, the delegate is dead code.

**Why:** Sprint 2 had `HandleHopCompleted` and `HandleFrogDied` methods that existed but were never bound. Score never updated, deaths weren't tracked.

### 14. Game Launch Commands — macOS (Added: Post-Sprint 2 Hotfix)

For visual play-testing on macOS:
```
"/Users/Shared/Epic Games/UE_5.7/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor" "<project>.uproject" -game -windowed -resx=1280 -resy=720 -log
```

Key rules:
- **Always use `-windowed`** — fullscreen crashes on macOS (SkyLight framework bug in UE 5.7)
- **Use `UnrealEditor.app`** (GUI) not `UnrealEditor-Cmd` (headless, no window)
- **`-game` uses the Editor binary** — always rebuild the Editor target before play-testing
- **Kill existing editor instances** before launching — shared memory conflicts cause silent startup failures
- Logs are at `~/Library/Logs/Unreal Engine/UnrealFrogEditor/`, NOT `Saved/Logs/`

### 15. Moving Platform Hop Convention (Added: Post-Sprint 2 Hotfix Retrospective)

When the frog is riding a moving platform (river log or turtle), the hop origin must be the frog's **actual world position**, not its stored `GridPosition`. After the hop completes, `FinishHop` must update `GridPosition` by back-calculating from the actual landing location.

Rules:
- `StartHop()`: if `CurrentPlatform != nullptr`, use `GetActorLocation()` as origin, not `GridToWorld(GridPosition)`
- `FinishHop()`: always update `GridPosition` from `WorldToGrid(GetActorLocation())`
- Never assume `GridPosition` is accurate while the frog is on a moving platform — the platform drifts the frog between hops

**Why:** The frog teleported sideways when hopping from a moving log because `StartHop` used the stale grid position (where the frog originally landed) instead of the current position (where the log had carried it). The frog would land in empty river and die instantly.

### 16. UE Runtime API Validation (Added: Sprint 4 Retrospective)

When using a new Unreal Engine API (especially for content that would normally be imported as .uassets), verify the approach works in a **Game target build** before writing full implementation.

Process:
1. Write a spike test (< 50 lines) that exercises the API
2. Build Game target and run the test
3. If it fails, research engine source or ask for guidance
4. Once validated, proceed with full implementation

**Why:** Sprint 4 burned 4 iterations discovering that `USoundWave::RawPCMData` is editor-only in UE 5.7. Only `USoundWaveProcedural` + `QueueAudio()` works for runtime-generated audio. A 10-minute spike would have caught this immediately.

### 17. Deferred Item Deadline (Added: Sprint 5 Retrospective)

**Any P1 action item deferred 3 consecutive sprints must be resolved in the next sprint: either completed or explicitly dropped.** When dropping, record the rationale in the retrospective log and remove from the carry-forward list. No more silent carry-forward.

**Why:** M_FlatColor.uasset and functional-tests-in-CI were deferred for 5 sprints. Chronic carry-forward items create noise in retrospectives and signal that the team is not honest about its priorities.

**Items dropped (Sprint 6 Retrospective):**
- **M_FlatColor.uasset**: DROPPED. Runtime `GetOrCreateFlatColorMaterial()` is sufficient for all current use cases. Persistent .uasset only needed for packaged builds, which are not on the roadmap. Re-create as P0 if packaging becomes a sprint goal.
- **Functional tests in CI**: DROPPED. 154 SIMPLE_AUTOMATION_TESTs run in NullRHI headless mode. AFunctionalTest actors require display server infrastructure for marginal gain. Revisit if visual regression testing is prioritized.

### 18. Cross-Domain Review Before Commit (Updated: Sprint 7 Retrospective)

**Before committing, the driver must post a summary of changes and receive review from an agent in a *different* domain.** The reviewer brings a perspective the driver lacks — this is the whole point. Same-domain review catches implementation bugs; cross-domain review catches design blind spots.

Cross-domain pairings (reviewer ← driver):
- **QA Lead** reviews **Engine Architect** (catches untested paths, edge cases)
- **Engine Architect** reviews **Game Designer** (catches performance implications of tuning values)
- **Game Designer** reviews **QA Lead** (catches tests that don't reflect real gameplay)
- **DevOps** reviews **Engine Architect** (catches build/CI implications)
- **Engine Architect** reviews **DevOps** (catches architectural assumptions in tooling)

The reviewer has one response cycle to approve or flag issues. Self-review is a last resort only when no other agent is available — and the commit message must note "self-reviewed."

**Why:** Sprint 6 shipped a seam test with incorrect floating-point boundary math. A same-domain reviewer might have missed it too. A cross-domain reviewer (Game Designer checking "does this test reflect real gameplay?") would have questioned the boundary values. (Evolved from Sprint 6's domain-expert-review rule.)

**XP Coach enforcement rule:** The XP Coach must not rationalize process exceptions. If an agreement is being violated, the XP Coach flags it and enforces. Exceptions require explicit team-lead approval, not silent acceptance. (Added: Sprint 7 Retrospective — XP Coach accepted a premature tuning change that violated §5 step 8; team-lead had to overrule.)

### 19. One Agent Runs Tests at a Time (Added: Sprint 7 Retrospective)

**Until run-tests.sh has a lock file mechanism, only one agent may run the test suite at a time.** The pre-flight `pkill -f UnrealEditor-Cmd` in run-tests.sh kills ALL editor processes, including another agent's in-progress test run. Coordinate test runs through the XP Coach.

Sprint 8 action: DevOps adds `mkdir`-based atomic lock file to run-tests.sh with `trap` cleanup on exit.

**Why:** Engine Architect reported persistent signal 15 kills during Sprint 7 when multiple agents attempted concurrent test runs.

### 20. PlayUnreal Must Support Scripted Gameplay (Updated: Sprint 8 Retrospective)

**The PlayUnreal tooling must support an agent writing a Python script that launches the game, sends hop commands, reads game state, and verifies outcomes.** The minimal API:
- `hop(direction)` — send a hop input to the running game
- `get_state()` — return `{score, lives, wave, frogPos, gameState}` as JSON
- `screenshot()` — save current frame to disk

Until PlayUnreal has been **verified working against a live game**, "QA: verified" in commit messages is only valid for code-level checks, not gameplay verification. **PlayUnreal is the team's highest priority.**

**Current state (Sprint 8):** Code exists but is UNVERIFIED. `Tools/PlayUnreal/client.py` (385 LOC), `acceptance_test.py` (159 LOC), and `run-playunreal.sh` (154 LOC) were committed in Sprint 8. `GetGameStateJSON()` was added to GameMode. RemoteControl plugins are enabled in .uproject. However, **none of this has been run against a live editor.** The acceptance test has never been executed. No screenshots have been taken. The code may or may not work.

**Implementation approach:** UE 5.7 Remote Control API plugin (localhost:30010) + Python client. `GetGameStateJSON()` UFUNCTION on GameMode for state queries.

**Acceptance criterion (unchanged):** A Python script that hops the frog across the road, across the river, and into a home slot -- executed against a real running game, producing real screenshots.

**Why:** Deferred since Sprint 3 (6 sprints). In Sprint 8, the code was written but never tested. The team has still never autonomously verified gameplay. Every stakeholder play-test finds bugs that 170+ automated tests missed.

### 21. Visual Commit Evidence Gate (Added: Sprint 8 Retrospective)

**Any commit that modifies visual output MUST include evidence of visual verification.** This is a HARD GATE enforced by the XP Coach. No exceptions.

"Visual output" includes: VFX effects (spawn, scale, position, duration), HUD elements (score pops, timer, wave announcements), materials/colors, camera changes, lighting, ground appearance. If you are unsure whether a change is visual, it is visual.

**Evidence** means ONE of the following committed alongside or immediately after the code commit:
1. **Screenshot** saved to `Saved/Screenshots/` showing the effect from the gameplay camera
2. **PlayUnreal script log** showing the effect was triggered and a screenshot was captured
3. **Explicit skip note** in the commit message: `visual verification: SKIPPED -- [reason]`. A SKIPPED commit is a draft that BLOCKS sprint completion (Section 5a step 4).

**What is NOT evidence:**
- A passing unit test (tests verify code logic, not visual output)
- An updated seam-matrix.md entry
- A statement in a message or transcript claiming the effect was verified
- A description of what the code is supposed to do

**Why:** Sprint 8 wrote 888 lines of VFX/HUD code. 170 tests passed. Zero screenshots were taken. The stakeholder confirmed the game looks identical to before the sprint. Writing code is not verifying code. Writing tests is not verifying visual output. Only looking at the running game verifies visual output.

**Items dropped (Sprint 8 Retrospective, per Section 17):**
- **Visual regression testing (7th deferral):** DROPPED. PlayUnreal screenshots provide screenshot-based verification naturally. Automated pixel-diff comparison is not needed when agents are not even taking screenshots.
- **Packaging step in CI (7th deferral):** DROPPED. No packaging sprint on the roadmap. Re-create if shipping build becomes a goal.
- **Rename PlayUnreal E2E (8th deferral):** DROPPED. Entire PlayUnreal architecture was rebuilt in Sprint 8. Naming is moot.

## Things We Will Figure Out Together

- ~~Optimal driver rotation timing~~ → Resolved: per-task, not time-based
- How to handle disagreements between agents
- When to bring in a specialist agent (Art Director, Sound Engineer, Level Designer not needed for Sprint 1)
- ~~How to balance speed vs. quality~~ → Resolved: quality gate is "playable and building" (Stakeholder Review)
- ~~Is mob programming viable?~~ → Resolved Sprint 6: solo driver with review gates. → **Reversed post-Sprint 6**: Stakeholder directive — always use agent teams, always get multiple perspectives. See Section 1.

## Resolved Questions (Sprint 1)

- **8 agents right?** Minimum 3 per task (xp-coach + domain expert + cross-domain challenger). Scale up as domains are touched. Art/Sound/Level agents join when content sprints begin.
- **Opus for all?** Engine Architect benefits from opus. DevOps scaffolding could use sonnet for speed. Evaluate per-task.
- **Driver rotation timing?** Per-task rotation works well. Engine Architect drove most implementation since it's all C++ systems.
- **WIP limits?** One feature at a time is sufficient. Allow parallel non-overlapping systems within a feature.
