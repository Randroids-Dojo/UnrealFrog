# Sprint 8 Plan: PlayUnreal Automation + Visual/Perception Fixes

*Created: Sprint 8 IPM (2026-02-15)*
*Facilitator: XP Coach*
*Participants: XP Coach, Engine Architect, Game Designer, DevOps Engineer, QA Lead*

## Goal

**Build the PlayUnreal automation harness and fix all visual/perception bugs -- so agents can autonomously verify gameplay and players can see the game's feedback systems.**

This sprint addresses the two longest-standing gaps in the project:
1. **PlayUnreal** (deferred since Sprint 3 -- 5 sprints). Agents have never been able to programmatically play the game.
2. **VFX/HUD visibility** (broken since Sprint 5 -- 3 sprints). Score pops, death VFX, and home celebrations are invisible or misplaced.

## Architecture Decision: Remote Control API

**Decided at IPM.** DevOps proposed, XP Coach validated, team consensus.

PlayUnreal uses UE 5.7's built-in **Remote Control API** plugin (`Engine/Plugins/VirtualProduction/RemoteControl/`). The plugin provides an HTTP server on `localhost:30010` that exposes all `UFUNCTION(BlueprintCallable)` and `UPROPERTY(BlueprintReadOnly)` values via REST endpoints. This eliminates the need for a custom C++ HTTP server (~200 LOC saved).

```
   Tools/PlayUnreal/              HTTP :30010              UnrealEditor
   client.py                 ---- PUT /remote/ ---->       + PIE (-game)
   acceptance_test.py        <--- JSON response ---        + RemoteControl
   visual_smoke_test.py                                    plugin enabled
```

**Spike required (Section 16 compliance):** Before committing to this approach, DevOps must verify Remote Control API responds to HTTP requests in `-game` mode on macOS. If the spike fails, fallback is a custom `UPlayUnrealServer` subsystem (~200 LOC C++), which reduces P1 scope.

**Minimal C++ additions:** One new UFUNCTION on `AUnrealFrogGameMode` (~30 LOC):
- `GetGameStateJSON()` -- returns combined state from GameMode + FrogCharacter + ScoreSubsystem as FString JSON
- Screenshot uses engine API (`FScreenshotRequest` / `UGameplayStatics`) directly -- no wrapper needed

*Decision rationale:* Engine Architect proposed adding `GetGameStateJSON()` directly to GameMode rather than creating a separate `UPlayUnrealHelpers` Blueprint Function Library. This is simpler (one UFUNCTION on an existing class vs a new class) and the function is approach-agnostic (works with Remote Control API or PythonScriptPlugin). The `TakeScreenshot` wrapper is unnecessary -- the engine provides this natively.

## Dependency Graph

```
Phase 0 (no deps, all parallel):
  Lock file (DevOps) ───────────────────────────────────────────► commit 1
  RC API spike (DevOps) ──► pass? ──► Phase 1A
  Temporal passability test (Eng Arch) ─────────────────────────► commit 2
  VFX/HUD Red tests (QA) ──► Phase 1B

Phase 1A (PlayUnreal, sequential):
  GetGameStateJSON on GM (Eng Arch) ──► Python client (DevOps) ──► acceptance test ──► launch script ──► commit 3

Phase 1B (VFX/HUD fixes, parallel with 1A):
  Score pop fix (Eng Arch) ──► commit 4a
  VFX visibility fix (Eng Arch) ──► commit 4b
  VFX home slot positions (Eng Arch) ──► commit 4c

Phase 2 (blocked on 1A + 1B):
  Visual verification via PlayUnreal (QA) ─────────────────────► (validation, no commit)

Phase 3 (blocked on PlayUnreal for verification):
  Audio pitch shift (Game Des) ──► commit 5a
  Wave fanfare ceremony (Game Des) ──► commit 5b
  Ground color temperature (Game Des) ──► commit 5c
  Gap reduction stretch (Eng Arch) ──► commit 5d (if capacity)

Phase 4 (blocked on all above):
  QA visual smoke test + sprint sign-off (QA) ─────────────────► commit 6
  Seam matrix update + Section 17 + retro (QA + XP Coach) ─────► commit 7

Note: All GameMode changes (Tasks 5, 13, 15, 15b) are sequential, same driver (Engine Architect
for 5/15b, Game Designer for 13/15), to respect one-writer-per-file rule.
```

## Phase 0: Foundation (No Dependencies)

All tasks can start immediately and run in parallel.

### Task 1: Lock File for run-tests.sh
- **Priority:** P0
- **Owner:** DevOps Engineer
- **Reviewer:** Engine Architect (cross-domain, Section 18)
- **Files:** `Tools/PlayUnreal/run-tests.sh`
- **LOC:** ~20
- **Description:** Add `mkdir`-based atomic lock file at `/tmp/.unrealFrog_test_lock` with PID tracking and `trap` cleanup on EXIT/INT/TERM. Prevents concurrent test runs from killing each other (Section 19).
- **Acceptance criteria:**
  - Running `run-tests.sh` twice simultaneously: second invocation exits with code 2 and prints "Another test run is in progress (PID: N)"
  - Lock is cleaned up on normal exit, Ctrl-C, and SIGTERM
  - Stale lock can be manually removed with `rm -rf`

### Task 2: Remote Control API Spike
- **Priority:** P0 (prerequisite for all PlayUnreal work)
- **Owner:** DevOps Engineer
- **Files:** `.uproject` (add RemoteControl plugin), spike test script
- **LOC:** ~15 (script + .uproject change)
- **Description:** Enable RemoteControl plugin in .uproject. Launch editor with `-game -windowed -ExecCmds="WebControl.StartServer"`. Curl `localhost:30010/remote/info`. Verify response. If successful, try `PUT /remote/object/call` on a known UFUNCTION.
- **Decision gate:** If spike PASSES, proceed with Phase 1A as planned. If spike FAILS, pivot to custom `UPlayUnrealServer` HTTP subsystem approach (~200 LOC C++) and reassess P1 scope (difficulty perception may be deferred to Sprint 9).
- **Acceptance criteria:**
  - HTTP request to `localhost:30010/remote/info` returns valid JSON
  - At least one `PUT /remote/object/call` successfully invokes a UFUNCTION
  - Object path discovery works (can find GameMode instance)

### Task 3: Temporal Passability Invariant Test
- **Priority:** P0
- **Owner:** Engine Architect
- **Reviewer:** QA Lead (cross-domain, Section 18)
- **Files:** `Source/UnrealFrog/Tests/SeamTest.cpp` (or new `Tests/PassabilityTest.cpp`)
- **LOC:** ~30
- **Description:** Mathematical invariant test: for every lane config, `HopDuration < MinGapCells * GridCellSize / (Config.Speed * GM->MaxSpeedMultiplier)`. Reads all tuning parameters from game objects at test time (tuning-resilient per Section 2). Encodes the 0.05s margin concern from Sprint 7 Task 6 review. The formula verifies that the frog can complete a hop before the trailing hazard arrives at max difficulty.
- **Current margin check:** HopDuration=0.15s, MinGap=200UU, MaxSpeed=500UU/s -> GapTime=0.4s -> margin=0.25s (OK). But tighter lanes or tuning changes could violate this.
- **Acceptance criteria:**
  - Test reads HopDuration, GridCellSize, MaxSpeedMultiplier from GM/FrogCharacter (not hardcoded)
  - Asserts passability for all lane configs at max difficulty
  - Reports the tightest lane and its margin
  - Fails if any tuning change creates an impossible lane

*Reassigned from QA Lead to Engine Architect per IPM discussion -- this is physics/timing analysis in the engine architect's domain.*

### Task 4: VFX/HUD Red Tests (Test-First)
- **Priority:** P0
- **Owner:** QA Lead
- **Reviewer:** Game Designer (cross-domain, Section 18)
- **Files:** `Source/UnrealFrog/Tests/HUDTest.cpp`, `Source/UnrealFrog/Tests/VFXTest.cpp`
- **LOC:** ~40 (3 tests)
- **Description:** Write failing tests FIRST (Red phase of TDD) that define the contract for the VFX/HUD fixes in Phase 1B:
  - `FHUD_ScorePopUsesWorldProjection` -- score pop position is NOT hardcoded to (20 + textLen*10, 10)
  - `FVFX_DeathPuffScaleForCameraDistance` -- death VFX EndScale produces >= 5% of screen width at Z=2200 (>= 103 UU diameter)
  - `FVFX_HomeSlotSparkleReadsGridConfig` -- home celebration positions derived from HomeSlotColumns, not magic numbers
- **Acceptance criteria:**
  - All 3 tests exist and FAIL against current code (Red)
  - Tests define quantitative thresholds, not subjective criteria
  - Tests are tuning-resilient (read camera height, FOV from configuration)

## Phase 1A: PlayUnreal Core (Sequential, Blocked on Spike)

### Task 5: PlayUnreal C++ Hook (GetGameStateJSON)
- **Priority:** P0
- **Owner:** Engine Architect (DevOps navigates)
- **Reviewer:** QA Lead (cross-domain, Section 18)
- **Files:** `Source/UnrealFrog/Public/Core/UnrealFrogGameMode.h`, `Source/UnrealFrog/Private/Core/UnrealFrogGameMode.cpp`
- **LOC:** ~30
- **Description:** Add `UFUNCTION(BlueprintCallable) FString GetGameStateJSON() const` to GameMode. Returns `{"score":N, "lives":N, "wave":N, "frogPos":[x,y], "gameState":"Playing", "timeRemaining":N, "homeSlotsFilledCount":N}`. Queries FrogCharacter via `UGameplayStatics::GetPlayerPawn()` and ScoreSubsystem via `GetGameInstance()->GetSubsystem<UScoreSubsystem>()`. No new files -- adds to existing GameMode.
- **Acceptance criteria:**
  - `GetGameStateJSON()` returns valid JSON with all specified fields
  - Tagged `UFUNCTION(BlueprintCallable)` for Remote Control API exposure
  - Handles null Frog/ScoreSubsystem gracefully (returns partial JSON, not crash)

*Architecture note: Engine Architect drives this since it touches GameMode internals. All GameMode changes (Tasks 5, 13, 15, and stretch Gap Reduction) are sequential, same driver, to avoid one-writer-per-file conflicts.*

### Task 6: PlayUnreal Python Client
- **Priority:** P0
- **Owner:** DevOps Engineer (Engine Architect navigates)
- **Reviewer:** Game Designer (cross-domain, Section 18)
- **Files:** `Tools/PlayUnreal/client.py`
- **LOC:** ~120
- **Description:** Python client using `urllib.request` (no pip deps). Class `PlayUnreal` with:
  - `hop(direction)` -- calls `AFrogCharacter::RequestHop(FVector)` via Remote Control
  - `get_state()` -- calls `AUnrealFrogGameMode::GetGameStateJSON()` via Remote Control, returns dict
  - `screenshot(path)` -- triggers `FScreenshotRequest` via Remote Control console command or helper
  - `reset_game()` -- calls `ReturnToTitle()` then `StartGame()` via Remote Control
  - `wait_for_state(target, timeout)` -- polls `get_state()` until state matches
  - Connection management with timeout and "editor not running" error handling
- **Acceptance criteria:**
  - All 5 methods work against a running editor with Remote Control enabled
  - Uses `urllib.request`, not `requests` (no pip dependency)
  - Timeout and connection error handling with clear error messages

### Task 7: PlayUnreal Acceptance Test
- **Priority:** P0
- **Owner:** DevOps Engineer
- **Reviewer:** QA Lead (cross-domain, Section 18)
- **Files:** `Tools/PlayUnreal/acceptance_test.py`
- **LOC:** ~60
- **Description:** The acceptance test from Section 20: a Python script that hops the frog across the road, across the river, and into a home slot.
  - `reset_game()` -> `wait_for_state("Playing")`
  - `hop("up")` x5 (cross safe zone + road)
  - `hop("up")` x5 (cross river on logs)
  - `hop("up")` x4 (reach home row)
  - Assert: score > 0, lives == 3, frog reached home slot
  - Take screenshot at start, middle, and end
- **Acceptance criteria:**
  - Script runs end-to-end without errors against a live editor
  - Assertions pass: score > 0, lives == 3, at least 1 home slot filled
  - Screenshots saved to `Saved/Screenshots/`

### Task 8: PlayUnreal Launch Script
- **Priority:** P0
- **Owner:** DevOps Engineer
- **Reviewer:** Engine Architect (cross-domain, Section 18)
- **Files:** `Tools/PlayUnreal/run-playunreal.sh`
- **LOC:** ~40
- **Description:** Shell script that:
  - Kills stale editor processes (same pattern as run-tests.sh)
  - Launches editor with `-game -windowed -resx=1280 -resy=720` + Remote Control enabled
  - Polls `localhost:30010/remote/info` until server responds (with timeout)
  - Runs the specified Python script
  - Captures exit code, kills editor on completion
- **Acceptance criteria:**
  - Successfully launches editor, waits for Remote Control, runs acceptance test, and shuts down
  - Returns exit code 0 on test pass, 1 on test fail, 2 on launch/timeout failure

## Phase 1B: VFX/HUD Fixes (Parallel with 1A)

All three tasks make the Red tests from Task 4 pass (Green phase of TDD).

### Task 9: Fix Score Pop Positioning
- **Priority:** P0
- **Owner:** Engine Architect
- **Reviewer:** QA Lead (cross-domain, Section 18)
- **Files:** `Source/UnrealFrog/Private/Core/FroggerHUD.cpp`
- **LOC:** ~25
- **Description:** Replace hardcoded `FVector2D(20 + textLen*10, 10)` with `GetOwningPlayerController()->ProjectWorldLocationToScreen(FrogLocation, ScreenPos)`. The frog reference comes from `UGameplayStatics::GetPlayerPawn()`. Score pops spawn at the frog's projected screen position and float upward. Includes null guards for frog and controller.
- **Acceptance criteria:**
  - `FHUD_ScorePopUsesWorldProjection` test passes (Green)
  - Score pop Position is derived from frog world position projection
  - Score pop floats upward over its lifetime (DeltaY < 0)

### Task 10: Fix VFX Visibility (Camera-Distance Scaling)
- **Priority:** P0
- **Owner:** Engine Architect
- **Reviewer:** Game Designer (cross-domain, Section 18)
- **Files:** `Source/UnrealFrog/Public/Core/FroggerVFXManager.h`, `Source/UnrealFrog/Private/Core/FroggerVFXManager.cpp`
- **LOC:** ~40
- **Description:** Add helper `CalculateScaleForScreenSize(FVector WorldLocation, float DesiredScreenFraction)` to VFXManager. The math: `DesiredWorldSize = 2.0 * CameraDistance * tan(FOV/2) * DesiredScreenFraction; Scale = DesiredWorldSize / MeshBaseSize`. Camera distance queried dynamically via `UGameplayStatics::GetPlayerCameraManager()` -- NOT hardcoded Z=2200, making the system resilient to camera changes. Apply to all VFX spawn calls:
  - SpawnDeathPuff: ~5% screen fraction, Duration 0.5s->0.8s (longer to register)
  - SpawnHopDust: ~2% screen fraction (subtle but visible)
  - SpawnHomeSlotSparkle: ~3% screen fraction
- **Acceptance criteria:**
  - `FVFX_DeathPuffScaleForCameraDistance` test passes (Green)
  - Death puff EndScale at Z=2200, FOV 50 produces >= 103 UU diameter
  - Hop dust and home sparkle also use camera-relative scaling
  - Duration unchanged (death: 0.5s, hop dust: 0.2s, sparkle: 1.0s)

### Task 11: Fix VFX Hardcoded Home Slot Positions
- **Priority:** P0
- **Owner:** Engine Architect
- **Reviewer:** QA Lead (cross-domain, Section 18)
- **Files:** `Source/UnrealFrog/Private/Core/FroggerVFXManager.cpp`
- **LOC:** ~20
- **Description:** Replace hardcoded `Col * 100, 14 * 100, 50` magic numbers in `SpawnRoundCompleteCelebration()` and `HandleHomeSlotFilled()` with positions queried from GameMode via `GetWorld()->GetAuthGameMode<AUnrealFrogGameMode>()`. Read `HomeSlotColumns`, `HomeSlotRow`, and `GridCellSize` to compute world positions. No new coupling needed -- VFXManager already has `GetWorld()`.
- **Acceptance criteria:**
  - `FVFX_HomeSlotSparkleReadsGridConfig` test passes (Green)
  - Celebration positions match HomeSlotColumns from GameMode
  - No literal grid coordinates in VFXManager.cpp

## Phase 2: Visual Verification (Blocked on 1A + 1B)

### Task 12: PlayUnreal Visual Verification of VFX/HUD Fixes
- **Priority:** P0
- **Owner:** QA Lead
- **Files:** No new files (uses PlayUnreal tools from Phase 1A)
- **Description:** Using PlayUnreal, verify all Phase 1B fixes are visible from the gameplay camera (Section 9 compliance):
  - [ ] Score pop appears near frog's screen position (not top-left corner)
  - [ ] Death VFX fills >= 5% of viewport
  - [ ] Home slot celebration appears at correct grid positions
  - [ ] All effects are visible at Z=2200 camera distance
- **Acceptance criteria:**
  - Screenshots taken for each VFX/HUD element
  - Each screenshot shows the effect at the expected screen location
  - QA Lead signs off: "QA: verified" for VFX/HUD commits

## Phase 3: Difficulty Perception (Blocked on PlayUnreal for Verification)

Implementation can start parallel with Phase 2, but verification requires PlayUnreal.

### Task 13: Audio Pitch/Tempo Shift Per Wave
- **Priority:** P1
- **Owner:** Game Designer (Engine Architect navigates)
- **Reviewer:** Engine Architect (cross-domain, Section 18)
- **Files:** `Source/UnrealFrog/Public/Core/FroggerAudioManager.h`, `Source/UnrealFrog/Private/Core/FroggerAudioManager.cpp`, `Source/UnrealFrog/Private/Core/UnrealFrogGameMode.cpp`
- **LOC:** ~10
- **Description:** `SetMusicPitchMultiplier(1.0 + (Wave-1) * 0.03)` called from `OnRoundCompleteFinished`. Uses `UAudioComponent::SetPitchMultiplier()` natively. By Wave 3: ~6% faster. Wave 8: ~21% faster.
- **Test:** `FSeam_WaveDifficultyProducesPerceptibleChange` verifies cumulative change >= 20% by Wave 3 (speed multiplier, not just pitch).
- **Acceptance criteria:**
  - Music playback rate increases audibly per wave
  - Test passes: wave 3 speed multiplier >= 1.20 relative to wave 1
  - Verified via PlayUnreal: can hear pitch difference between Wave 1 and Wave 3

### Task 14: Wave Fanfare Ceremony
- **Priority:** P1
- **Owner:** Game Designer (Engine Architect navigates)
- **Reviewer:** QA Lead (cross-domain, Section 18)
- **Files:** `Source/UnrealFrog/Private/Core/FroggerHUD.cpp`, `Source/UnrealFrog/Public/Core/FroggerHUD.h`, `Tools/AudioGen/generate_sfx.py`
- **LOC:** ~50
- **Description:** Wave transitions get a 1.5s ceremony: "WAVE N" text animates from 200% scale to 100% with screen flash. Ascending 3-note jingle (generated via `generate_sfx.py` or uses existing `RoundCompleteSound` as placeholder). Occurs during existing 2.0s `RoundCompleteDuration`.
- **Acceptance criteria:**
  - "WAVE N" text is centered on screen, animates scale from 200% to 100%
  - Duration fits within existing RoundCompleteDuration (no gameplay timing change)
  - Verified via PlayUnreal screenshot at 0.5s into wave transition

### Task 15: Ground Color Temperature Per Wave
- **Priority:** P1
- **Owner:** Game Designer (Engine Architect navigates)
- **Reviewer:** Engine Architect (cross-domain, Section 18)
- **Files:** `Source/UnrealFrog/Public/Core/GroundBuilder.h`, `Source/UnrealFrog/Private/Core/GroundBuilder.cpp`, `Source/UnrealFrog/Private/Core/UnrealFrogGameMode.cpp`
- **LOC:** ~40
- **Description:** Ground/safe zone color shifts from cool to warm per wave. Wave 1: green/blue. Wave 3: yellow. Wave 5: orange. Wave 7+: red. `UpdateWaveColor(WaveNumber)` called from `OnRoundCompleteFinished`. Dynamic material color lerp.
- **Acceptance criteria:**
  - Ground color visibly changes between Wave 1 and Wave 3 (passes "5-second test")
  - Verified via PlayUnreal: screenshots at Wave 1, 3, 5 show distinct color temperatures

### Task 15b: Gap Reduction via Wrap Tightening (Stretch)
- **Priority:** P1 (stretch -- first P1 to cut if sprint runs long)
- **Owner:** Engine Architect
- **Reviewer:** QA Lead (cross-domain, Section 18)
- **Files:** `Source/UnrealFrog/Public/Core/HazardBase.h`, `Source/UnrealFrog/Private/Core/HazardBase.cpp`, `Source/UnrealFrog/Private/Core/LaneManager.cpp`
- **LOC:** ~40
- **Description:** `GetGapReduction()` computes a value but it's never consumed. Implement the "tighten wrap" approach: add `WrapPadding` UPROPERTY to HazardBase (default = GridCellSize). In `ApplyWaveDifficulty`, reduce `WrapPadding` by `GapReduction * GridCellSize * 0.5`, causing hazards to wrap sooner and appear more frequently in the play field. This avoids spawning new actors mid-wave.
- **Safety net:** Temporal passability test (Task 3) validates that tightened gaps remain passable at all difficulty levels.
- **Acceptance criteria:**
  - `GetGapReduction()` value is consumed by `ApplyWaveDifficulty()`
  - Hazards appear more frequently at higher waves (visually denser lanes)
  - Temporal passability test still passes after gap reduction is applied
  - No new hazard actors spawned mid-wave

*Added per Engine Architect proposal. Sequential with Task 5 (both touch GameMode via wiring in OnRoundCompleteFinished). Same driver handles all GameMode changes to avoid file conflicts.*

### "5-Second Test" (Acceptance Criterion for All Phase 3 Tasks)

Show a player 5 seconds of Wave 1 gameplay and 5 seconds of Wave 3 gameplay back-to-back. If they cannot immediately tell which is which, the perception feedback is insufficient. PlayUnreal screenshots at Wave 1 and Wave 3 must show visually distinct color temperature, and audio playback must be audibly faster.

## Phase 4: Sprint Sign-Off

### Task 16: QA Visual Smoke Test
- **Priority:** P1
- **Owner:** QA Lead
- **Files:** `Tools/PlayUnreal/visual_smoke_test.py`
- **LOC:** ~80
- **Description:** Python script using PlayUnreal that triggers every VFX/HUD element and takes screenshots:
  - Hop forward (score pop) -> screenshot
  - Get hit by car (death VFX + death flash) -> screenshot
  - Fill home slot (celebration VFX) -> screenshot
  - Timer runs low (timer pulse) -> screenshot
  - Reach Wave 2 (wave announcement + audio pitch + ground color) -> screenshot
  - Save to `Saved/Screenshots/smoke_test_YYYY-MM-DD/`
- **Acceptance criteria:**
  - Script runs end-to-end without errors
  - Screenshots capture each VFX/HUD element at its expected screen location
  - QA Lead verifies: all effects visible, positioned correctly, timed correctly

### Task 17: Seam Matrix Update
- **Priority:** P1
- **Owner:** QA Lead
- **Files:** `Docs/Testing/seam-matrix.md`
- **LOC:** ~15
- **Description:** Add 6 new seam entries (seams 20-25):
  - PlayUnreal -> GameMode state query accuracy
  - PlayUnreal -> FrogCharacter hop command
  - FroggerHUD -> APlayerController score pop projection
  - FroggerVFXManager -> Camera distance scaling
  - FroggerVFXManager -> GameMode home slot grid config
  - Difficulty scaling -> human perception threshold
- **Acceptance criteria:**
  - All 6 entries have status COVERED with test names, or DEFERRED with rationale

### Task 18: Section 17 Resolution + Retrospective Prep
- **Priority:** P2
- **Owner:** XP Coach
- **Files:** `.team/agreements.md`, `.team/retrospective-log.md`
- **Description:** Drop 3 chronic P2 carry-forwards per Section 17:
  - Visual regression testing (6th deferral): DROP -- PlayUnreal screenshots provide this capability naturally
  - Packaging step in CI (6th deferral): DROP -- no packaging sprint on roadmap
  - Rename PlayUnreal E2E (7th deferral): DROP -- entire PlayUnreal architecture rebuilt, naming moot
- **Acceptance criteria:**
  - Each dropped item has recorded rationale in retrospective log
  - Agreements updated to reflect drops

## Explicitly Deferred to Sprint 9

| Item | Original Priority | Reason for Deferral |
|------|-------------------|---------------------|
| UFroggerGameCoordinator extraction | P1 | Internal refactor, not player-facing. Sprint 8 scope is full. Also conflicts with GameMode changes this sprint. |
| Speed lines overlay (Signal 4) | P1 | Stretch. Build after signals 1-3 are verified. |
| Hazard color intensification (Signal 5) | P1 | Stretch. Build after ground color temperature validates. |
| Multiplier visibility on HUD | Stretch | Only if Sprint 8 P0+P1 complete early. |
| Death freeze frame + screen shake | Stretch | Only if Sprint 8 P0+P1 complete early. |
| A/B tuning sweep script | P1 | Depends on PlayUnreal + all perception signals. Sprint 9. |

## Risk Register

| # | Risk | Likelihood | Impact | Mitigation |
|---|------|-----------|--------|------------|
| 1 | Remote Control API fails in -game mode | Medium | High (pivot to custom HTTP ~200 LOC) | Spike first (Task 2). If fails, reassess P1 scope. |
| 2 | PlayUnreal scope creep beyond 2 sessions | Medium | Medium (squeezes VFX/HUD time) | Time-box to 2 sessions. Ship VFX with "QA: visual pending" if needed. |
| 3 | Object path discovery in PIE mode | Medium | Medium (Python client can't find objects) | GetGameStateJSON helper handles discovery internally. |
| 4 | VFX scale math error (mesh base extent unknown) | Low | Low (test catches it) | TDD -- QA writes Red test with quantitative threshold first. |
| 5 | Port 30010 conflict from stale editor | Low | Low (launch script handles it) | pkill stale editors in launch script, same as run-tests.sh. |
| 6 | Temporal passability 0.05s single-frame margin | Medium | High at max difficulty | Test flags it; PlayUnreal verifies at runtime with real frame timing. |
| 7 | Monolithic PlayUnreal commit | Low | Process violation | Per-subsystem commits: C++ helpers, Python client, acceptance test, launch script as separate commits. |
| 8 | ProjectWorldLocationToScreen returns zeros in NullRHI/headless | Medium | Score pop test needs workaround | Test verifies math/logic separately; visual verify via PlayUnreal screenshot. Flag from Engine Architect. |

## Process Reminders

- **Section 19:** One agent runs tests at a time. Lock file (Task 1) lands first.
- **Section 18:** Cross-domain review before every commit. Pairings assigned per task.
- **Section 4:** Per-subsystem commits. Target: 7+ commits for this sprint.
- **Section 5 step 9:** QA play-test BEFORE sprint commit. Visual verification in Phase 2.
- **Section 9:** Visual smoke test after VFX/HUD fixes. For real this time -- launch the game.
- **Section 20:** PlayUnreal acceptance test must pass before sprint is done.
- **Section 16:** Spike-test Remote Control API before full implementation.
- **Section 2:** TDD for all new code. QA writes Red tests for VFX fixes, implementer writes Green.
- **Section 1:** Active navigation during implementation. Navigator questions design, flags edge cases.

## Estimated Test Count

| Category | Current | Sprint 8 New | Sprint 8 Total |
|----------|---------|-------------|----------------|
| Seam tests | 19 | 2-3 (temporal passability, perception threshold, PlayUnreal state) | 21-22 |
| VFX tests | 8 | 2 (camera scale, home slot config) | 10 |
| HUD tests | 5 | 1 (score pop projection) | 6 |
| PlayUnreal tests | 5 | 1-2 (state roundtrip, death recovery) | 6-7 |
| Other existing | 125 | 0 | 125 |
| **Total** | **162** | **6-9** | **168-171** |

## Driver Assignment Summary

| Agent | Tasks | Role |
|-------|-------|------|
| DevOps Engineer | 1 (lock file), 2 (spike), 6-8 (PlayUnreal Python + script) | Driver |
| Engine Architect | 3 (passability test), 5 (GetGameStateJSON), 9-11 (VFX/HUD Green fixes), 15b (gap reduction, stretch) | Driver |
| QA Lead | 4 (Red tests), 12 (visual verification), 16 (smoke test), 17 (seam matrix) | Driver |
| Game Designer | 13-15 (difficulty perception signals 1-3) | Driver |
| XP Coach | 18 (Section 17 + retro prep), IPM facilitation, process enforcement | Coordinator |

## Contingency Plan

If the Remote Control API spike (Task 2) **fails**:
1. DevOps pivots to custom `UPlayUnrealServer` C++ HTTP subsystem (~200 LOC)
2. Phase 1A timeline extends by ~1 session
3. Difficulty perception signals (Phase 3) are deferred to Sprint 9 to stay within scope
4. Sprint 8 delivers: lock file + PlayUnreal (custom HTTP) + VFX/HUD fixes + visual verification
5. Sprint 8 goal narrows to: "Build PlayUnreal and fix visual bugs"
