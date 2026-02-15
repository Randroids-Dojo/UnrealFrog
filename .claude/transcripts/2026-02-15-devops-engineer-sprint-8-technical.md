# Conversation: 2026-02-15

| Field | Value |
|-------|-------|
| Date | 2026-02-15 |
| Session ID | `247322c9-2ef2-44f0-a3c1-77b896924763` |
| Model | claude-opus-4-6 |
| Tokens | ~1,054,741 input, ~60 output |
| Duration | 50s |

---

## User (17:53 UTC)

<teammate-message teammate_id="devops-engineer" color="yellow" summary="Sprint 8 DevOps proposal: PlayUnreal architecture + tooling">
## DevOps Engineer -- Sprint 8 Technical Proposal

### Task 1: P0 -- Build PlayUnreal Python Automation

**Revised architecture recommendation:** After reviewing the engine spike research from Sprint 2, the stakeholder feedback, and the available UE 5.7 infrastructure, I am changing my recommendation from PythonScriptPlugin-only to **Remote Control API + Python client**.

**Why the change from PythonScriptPlugin:** The Sprint 2 spike documented a critical finding: PythonScriptPlugin runs IN-PROCESS in the editor's Python interpreter. It is great for asset setup (we already use it for `create_frog_map.py`), but it has a fundamental limitation: **it cannot start PIE from Python** (no exposed binding). The whole point of PlayUnreal is to script a *running game* from an external process -- send hops, read state, take screenshots. In-process Python cannot do this because it shares the game thread and blocks Tick.

Remote Control API is the correct approach:

```
Architecture:

   ┌──────────────────────┐         HTTP :30010        ┌───────────────────────┐
   │  Tools/PlayUnreal/   │  ───── PUT /remote/ ─────► │  UnrealEditor         │
   │  client.py           │  ◄──── JSON response ───── │  + PIE (-game flag)   │
   │                      │                            │  + RemoteControl      │
   │  hop("up")           │  PUT /remote/object/call   │  plugin enabled       │
   │  get_state()         │  PUT /remote/object/prop   │                       │
   │  screenshot("x.png") │  PUT /remote/preset/...    │  All UPROPERTY +      │
   │  reset_game()        │                            │  UFUNCTION exposed    │
   └──────────────────────┘                            └───────────────────────┘
```

**How each API function works:**

1. **`hop(direction)`**: Calls `AFrogCharacter::RequestHop(FVector)` via `PUT /remote/object/call`. The frog already has `UFUNCTION(BlueprintCallable)` on `RequestHop`, so the Remote Control API exposes it automatically. No C++ changes needed for this function.

2. **`get_state()`**: Reads `UPROPERTY(BlueprintReadOnly)` values via `PUT /remote/object/property`. GameMode has `CurrentState`, `CurrentWave`, `HomeSlotsFilledCount`, `RemainingTime`. FrogCharacter has `GridPosition`, `bIsDead`, `bIsHopping`. ScoreSubsystem has `Score`, `Lives`, `Multiplier`. All already tagged correctly for exposure.

3. **`screenshot(path)`**: UE console command `HighResShot` captures framebuffer. Triggered via `PUT /remote/object/call` on a small helper UFUNCTION, OR via `-ExecCmds` on a console command proxy. Approximately 15 LOC of C++ for a helper if the console command route is awkward.

4. **`reset_game()`**: Calls `AUnrealFrogGameMode::ReturnToTitle()` then `StartGame()`. Both already `UFUNCTION(BlueprintCallable)`.

**What needs C++ work (minimal):**

One new small class: `UPlayUnrealHelpers` -- a Blueprint Function Library (~40 LOC) with:
- `UFUNCTION(BlueprintCallable) static void TakeScreenshot(const FString& Path)` -- wraps `FScreenshotRequest`
- `UFUNCTION(BlueprintCallable) static FString GetGameStateJSON()` -- returns combined state from GM + Frog + Score as a single JSON blob (convenience for the Python client, avoids 3 separate HTTP calls)

**What needs Python work:**

`Tools/PlayUnreal/client.py` (~120 LOC):
- `PlayUnreal` class wrapping `requests` library (or raw `urllib` to avoid pip deps)
- Methods: `hop(direction)`, `get_state()`, `screenshot(path)`, `reset_game()`, `wait_for_state(target, timeout)`
- Connection management: constructor takes `host="localhost", port=30010`
- Error handling: timeout, connection refused (editor not running), JSON parse errors

`Tools/PlayUnreal/acceptance_test.py` (~60 LOC):
- The acceptance test from §20: hop across road, across river, into home slot
- Uses `wait_for_state("Playing")` between actions
- Asserts score > 0, lives == 3, frog reached row 14

`Tools/PlayUnreal/run-playunreal.sh` (~40 LOC):
- Launches editor with `-game -windowed` + Remote Control enabled
- Waits for HTTP server to respond (poll `localhost:30010`)
- Runs the Python acceptance test
- Captures exit code, kills editor on completion

**Plugin dependency:** Add `"RemoteControl"` to `.uproject` Plugins array (Enabled: true). Verified: plugin exists at `Engine/Plugins/VirtualProduction/RemoteControl/`, supports Mac, provides HTTP + WebSocket server on port 30010.

**Estimated LOC:**
- C++ UPlayUnrealHelpers: ~40 LOC (1 header + 1 implementation)
- Python client.py: ~120 LOC
- Python acceptance_test.py: ~60 LOC
- Shell run-playunreal.sh: ~40 LOC
- .uproject change: 3 lines
- Build.cs change: 0 (helpers use Engine APIs only, no new module deps)
- Total: **~260 LOC across 5 files**

**Risks:**

1. **Remote Control API may not expose objects during `-game` mode.** The plugin is documented for "Editor or Runtime" but most usage is in-editor. **Spike needed:** 30-minute test -- enable the plugin, launch with `-game`, curl `localhost:30010/remote/info`. If it responds, we are good. If not, fall back to `-ExecCmds` + custom `UPlayUnrealServer` HTTP subsystem (~200 LOC more).

2. **Object path discovery.** Remote Control API needs the object path to call functions (e.g., `/Game/Maps/FroggerMain.FroggerMain:PersistentLevel.UnrealFrogGameMode_0`). During PIE these get `UEDPIE_X_` prefixes. The Python client needs a discovery step (`PUT /remote/search/assets`) or we hardcode known paths.

3. **Port conflict.** If the editor is already running (from another agent or a previous crash), port 30010 may be occupied. The launch script must pkill stale editors first (same pattern as run-tests.sh).

4. **No `requests` in system Python.** macOS system Python 3 does not include `requests`. Use `urllib.request` from stdlib to avoid pip dependency.

**Spike before full implementation:** I strongly recommend a 30-minute spike (P0-prerequisite) to verify Remote Control API works in `-game` mode on macOS with our project. This follows our §16 (UE Runtime API Validation). If the spike fails, I have a backup plan: a lightweight C++ HTTP server using `FHttpServerModule` (~200 LOC) that exposes the same REST interface without the Remote Control plugin.

---

### Task 2: P0 -- Lock File for run-tests.sh

Straightforward. Add to `run-tests.sh`:

```bash
LOCK_DIR="/tmp/.unrealFrog_test_lock"

acquire_lock() {
    if ! mkdir "${LOCK_DIR}" 2>/dev/null; then
        LOCK_PID=$(cat "${LOCK_DIR}/pid" 2>/dev/null || echo "unknown")
        echo "ERROR: Another test run is in progress (PID: ${LOCK_PID})"
        echo "       Lock: ${LOCK_DIR}"
        echo "       If stale, remove with: rm -rf ${LOCK_DIR}"
        exit 2
    fi
    echo $$ > "${LOCK_DIR}/pid"
    trap release_lock EXIT INT TERM
}

release_lock() {
    rm -rf "${LOCK_DIR}" 2>/dev/null || true
}
```

Insert `acquire_lock` call after validation, before pre-flight pkill. The `trap` ensures cleanup on exit, interrupt, or termination.

**Estimated LOC:** ~20 lines added to run-tests.sh.

---

### Task 3: P1 -- Visual Smoke Test via PlayUnreal

Depends on Task 1 completing. Once PlayUnreal works:

`Tools/PlayUnreal/visual_smoke_test.py` (~80 LOC):
- Launch game, wait for Playing state
- Trigger each VFX/HUD element via gameplay actions:
  - Hop forward (score pop), screenshot
  - Get hit by car (death VFX + death flash), screenshot
  - Fill home slot (celebration VFX), screenshot
  - Let timer run low (timer pulse), screenshot
  - Reach Wave 2 (wave announcement), screenshot
- Save screenshots to `Saved/Screenshots/smoke_test_YYYY-MM-DD/`
- No automated comparison (that is P2 visual regression) -- just capture for human review

**Estimated LOC:** ~80 LOC Python.

---

### Task 4: P2 -- Section 17 Deadline on Carry-Forwards

My recommendation for the three P2 items that have hit the 3-sprint deadline:

1. **Visual regression testing (5th deferral):** **DROP.** We do not have a baseline to compare against. PlayUnreal screenshots are the prerequisite. Once PlayUnreal ships and we have a sprint's worth of screenshots, visual regression becomes meaningful. Re-create as P1 when screenshot baseline exists.

2. **Packaging step in CI (5th deferral):** **DROP.** No CI pipeline exists. Packaging is not on the roadmap. `WITH_EDITORONLY_DATA` issues have not caused any bugs since Sprint 2 (the FlatColor fallback works). Re-create as P0 when packaging becomes a sprint goal.

3. **Rename PlayUnreal E2E to "Scenario" (6th deferral):** **DROP.** The test file is `PlayUnrealTest.cpp` with test paths `UnrealFrog.PlayUnreal.ScenarioN_*`. The "Scenario" naming is already in the test names. Renaming the file gains nothing. The file serves as scenario/integration tests and the name is fine.

---

### Opinion: Sprint Scope

**PlayUnreal should be the primary focus, but VFX/HUD fixes CAN happen in parallel** -- they touch zero overlapping files:

- PlayUnreal touches: `.uproject`, new `PlayUnrealHelpers.h/.cpp`, new Python files, `run-tests.sh`
- VFX fixes touch: `FroggerVFXManager.h/.cpp`, `FroggerHUD.h/.cpp`
- Zero file overlap. Two drivers can work simultaneously per §1.

**Recommended sequence:**
1. **Phase 0 (day 1):** Lock file fix (DevOps, 20 min) + Remote Control API spike (DevOps, 30 min)
2. **Phase 1 (parallel):** PlayUnreal C++ helpers + Python client (DevOps) || VFX/HUD fixes (Engine Architect)
3. **Phase 2:** Visual smoke test using PlayUnreal to verify VFX fixes are visible (QA Lead)
4. **Phase 3:** Cross-domain review + commit

This way PlayUnreal enables verification of the VFX fixes in the same sprint, which is the whole point.

---

### Dependencies

```
Lock file fix ──────────────────────────────────────► commit 1
Remote Control spike ──► PlayUnreal C++ ──► Python client ──► acceptance test ──► commit 2
                                         VFX/HUD fixes ──────────────────────────► commit 3
                                         PlayUnreal + VFX ──► visual smoke test ──► commit 4
```

### Summary Table

| Task | Priority | LOC | Owner | Dependencies | Risk |
|------|----------|-----|-------|-------------|------|
| Lock file for run-tests.sh | P0 | ~20 | DevOps | None | None |
| Remote Control API spike | P0-prereq | ~10 | DevOps | None | Medium (may not work in -game mode) |
| PlayUnreal C++ helpers | P0 | ~40 | DevOps | Spike passes | Low |
| PlayUnreal Python client | P0 | ~120 | DevOps | C++ helpers built | Low |
| PlayUnreal acceptance test | P0 | ~60 | DevOps | Python client | Low |
| PlayUnreal launch script | P0 | ~40 | DevOps | Python client | Low |
| Visual smoke test | P1 | ~80 | QA Lead | PlayUnreal complete | Low |
| §17 P2 drops | P2 | 0 | XP Coach | None | None |

</teammate-message>

## Assistant (17:53 UTC)

This is excellent input from DevOps. The Remote Control API approach is a significant pivot from the stakeholder review's "C++ HTTP server" proposal. Let me evaluate it against the alternatives and respond.

- **Used Grep** for `RemoteControl` in `.`

- **Used Bash**: Check if RemoteControl plugin exists in UE 5.7

- **Used Read** on `/Users/Shared/Epic Games/UE_5.7/Engine/Plugins/VirtualProduction/RemoteControl/RemoteControl.uplugin`

- **Used Bash**: List RemoteControl plugin source modules

## Assistant (17:54 UTC)

Good. The RemoteControl plugin is confirmed: it exists, supports Mac, has Runtime modules (not Editor-only), and the `.uplugin` description explicitly says "in Editor or at Runtime." The `WebRemoteControl` module (the HTTP server) has a Mac platform allow-list entry. This is a much better approach than writing a custom HTTP server.

- **Used SendMessage**

## Assistant (17:54 UTC)

I'm actively waiting for team-lead's go-ahead or engine-architect's technical input before writing the final plan. Let me check if there are any new messages by reviewing the task state.

- **Used TaskList**

## Assistant (17:54 UTC)

Still waiting for responses. I have solid input from 3 agents now (XP Coach analysis, Game Designer design, DevOps architecture). The engine-architect questions are important for implementation details but the sprint scope and phasing are clear enough to write the plan. Let me send a final check to team-lead.

- **Used SendMessage**

## Assistant (17:54 UTC)

Standing by. I have the full plan structure ready in my head -- once I get the go-ahead, I can write `Docs/Planning/sprint8-plan.md` quickly.
