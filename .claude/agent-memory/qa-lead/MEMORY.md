# QA Lead Memory

*Persistent knowledge accumulated across sessions. First 200 lines loaded automatically.*

## Sprint 9 Visual Verification (3 Runs)

**On 2026-02-15, ran verify_visuals.py 3 times against live games. First automated visual verification in project history.**

### Run 3 Results (with hazard queries + invincibility)
- **WORKING**: Ground, frog, camera, HUD (score/timer/lives), lighting, hazards, lane zones, state text (GET READY, GAME OVER), title screen, gameplay loop, **home slot filling** (all 5 slots), score accumulation (660+), timer bar color change
- **BROKEN (NOT VISIBLE)**: Death puff VFX, Score pops +N text, Hop dust VFX, Home celebration VFX (sparkle/flash)
- **UNTESTED**: Wave fanfare, ground color change (wave stayed at 1 despite 5 home fills)
- **Wave issue**: homeSlotsFilledCount increased 5 times but wave never changed from 1. Possible game logic issue — does wave only increment after a specific RoundComplete transition?
- 22 screenshots + 4 videos in `Saved/Screenshots/smoke_test/`

### PlayUnreal is OPERATIONAL
- RC API connects, live object paths: GameMode_0, FrogCharacter_0
- hop(), get_state(), screenshot(), record_video(), set_invincible(), get_hazards() all work
- run-playunreal.sh: editor launches in ~6-8s, 3s actor spawn wait
- SetInvincible + hazard-query navigation = reliable full-field traversal

### Key Script Patterns
- `ensure_playing()` must invalidate `pu._frog_path = None` after reset (actor recreated)
- `hop_to_home_slot()` uses `_wait_for_safe_moment()` with hazard queries at 20Hz
- GameOver handling: log without `check()` to avoid failure spam; single summary check
- After ensure_playing, wait 0.5s before first hop (frog ignores input during spawn animation)

## Corrective Actions (MANDATORY for all future sprints)
1. Run `qa_checklist.py` against a live game before ANY sign-off
2. If qa_checklist.py cannot run, commit message MUST say "QA: code-level only, visual PENDING"
3. NEVER write "QA: verified" without: (a) qa_checklist.py PASS, or (b) reviewed screenshots
4. qa_checklist.py is at: `/Tools/PlayUnreal/qa_checklist.py`
5. verify_visuals.py is at: `/Tools/PlayUnreal/verify_visuals.py` (11 steps, saves to smoke_test/)

## Sprint 8 Plan
- **Sprint 8 goal**: PlayUnreal automation (P0), VFX/HUD visibility fixes, visual verification
- **Test count at sprint start**: 162 passing (0 failures)
- **Seam matrix**: 19 entries (14 COVERED, 5 DEFERRED)

### VFX Visibility Acceptance Criteria (Quantitative)
- Death VFX: >= 5% of visible screen width at Z=2200/FOV50 (~103 UU diameter)
- Score pops: within 200px of frog projected screen position, >= 24pt font, >= 0.8s duration
- Home celebration: at correct world position from HomeSlotColumns, >= 4 sparkle actors, >= 1.0s
- Wave announcement: centered on screen (within 10%), >= 36pt, >= 1.5s

## Sprint 7 State (completed)
- 162 total tests, 0 failures across 17 categories
- 5 commits for 5 subsystems (per-subsystem compliance)
- Sprint goal: Consolidation -- play-test, tune, fix. No new mechanics.
- QA: code-level verified. Visual play-test PENDING (carried to Sprint 8 P0).
- Tuning changes: DifficultySpeedIncrement 0.1->0.15, InputBufferWindow 0.1->0.08
- Dead code fixed: difficulty wiring (GetSpeedMultiplier consumed), InputBufferWindow enforced
- SaveHighScore: now only at game over / title transition
- Duplicate wave-complete: consolidated to TryFillHomeSlot as single authority

## Sprint 6 State (completed)
- 24 test files, 154 total tests, 0 failures (after boundary math fix)
- 5 commits for 4 subsystems -- first sprint with proper per-subsystem commits
- Sprint goal: Polish fixes -- TickVFX wiring, score pop positioning, high score persistence, death flash, timer warning sound
- Visual launch verified: no crashes, lane system initialized, clean shutdown
- Full gameplay play-test NOT completed (2nd consecutive sprint)

## Sprint 6 Findings

### Test Bug: TimerWarningFiresAtThreshold (SeamTest.cpp:612-616)
- Test ticks 25.0s, expects warning NOT fired (thinks 5.0/30.0 = 0.1667 > 0.167)
- Actually: 5.0/30.0 = 0.16666... < 0.167, so warning fires immediately
- Fix: change tick from 25.0f to 24.9f (RemainingTime=5.1, percent=0.17 > 0.167)
- Game implementation is CORRECT. Only test math is wrong.

### Sprint 5 Issues Resolved in Sprint 6
- TickVFX dead code: NOW WIRED in GameMode::Tick (line 132)
- Score pop hardcoded position: NOW PROPORTIONAL (20 + ScoreText.Len() * 10)
- VFX TickVFX seam: NOW COVERED by FSeam_VFXTickDrivesAnimation

### Outstanding Issues (Carried Forward)
- Music looping unverified: bLooping=true with one-shot QueueAudio() -- needs manual play-test
- HUD tests don't exercise DrawHUD (no Canvas in NewObject tests)
- VFX tests only verify null safety (no world context for actual spawning tests)
- No LaneManager actual spawning test
- No input-to-movement pipeline test

## Defect Escape Rate Trend
- Sprint 2: 5% (4 critical bugs from 81 tests)
- Sprint 3: 2% (2 bugs from 101 tests)
- Sprint 4: 1.6% (2 bugs from 123 tests)
- Sprint 5: UNKNOWN (play-test not done)
- Sprint 6: 0.65% (1 test bug from 154 tests, 0 game bugs found)

## Seam Matrix Current State (Sprint 6)
- 19 entries: 14 COVERED, 5 DEFERRED
- Sprint 6 added seams 18-19 (VFX+GM Tick, AudioManager+GM Timer)
- All deferred items have documented rationale

## Key File Locations (updated Sprint 6)
- VFX Manager: /Source/UnrealFrog/Public/Core/FroggerVFXManager.h, Private/Core/FroggerVFXManager.cpp
- Audio Manager: /Source/UnrealFrog/Public/Core/FroggerAudioManager.h, Private/Core/FroggerAudioManager.cpp
- HUD: /Source/UnrealFrog/Public/Core/FroggerHUD.h, Private/Core/FroggerHUD.cpp
- GameMode wiring: /Source/UnrealFrog/Private/Core/UnrealFrogGameMode.cpp
  - VFX wiring: lines 98-117
  - TickVFX call: line 132
  - Timer warning: lines 278-290
- Score persistence: /Source/UnrealFrog/Private/Core/ScoreSubsystem.cpp (LoadHighScore/SaveHighScore)
- High score file: Saved/highscore.txt
- Seam matrix: /Docs/Testing/seam-matrix.md
- Tests: /Source/UnrealFrog/Tests/ (24 files)
- Agreements: /.team/agreements.md
- Retro log: /.team/retrospective-log.md

## Floating Point Boundary Testing Lesson (Sprint 6)
When testing threshold crossings (e.g., "fire at < 16.7%"), compute exact decimal values:
- 5.0/30.0 = 0.16666... (BELOW 0.167, not above)
- 5.01/30.0 = 0.167 (AT threshold)
- 5.1/30.0 = 0.17 (ABOVE threshold)
Always pick a value clearly on the correct side with margin, not at the mathematical edge.

## Spec-vs-Implementation Mismatches Found (Sprint 2 IPM)
10 mismatches identified between sprint1-gameplay-spec.md and current code/tests:
1. Game state enum: spec has 6 states, code has 4 (missing Spawning, Dying, RoundComplete; has Paused which spec defers)
2. Multiplier: spec says +1 per hop (1x,2x,3x,4x,5x cap), code uses +0.5f
3. MaxLives: spec says 9, code says 5
4. Time bonus: spec says RemainingSeconds*10, code uses (Remaining/Max)*1000
5. Round-complete 1000 bonus: spec requires it, no code/test exists
6. WavesPerGapReduction: spec says every 2 waves, code says every 3
7. Speed cap at 2.0x: spec requires it, no code enforces it
8. GridToWorld offset: spec uses (Col-6)*100 offset, code does not apply offset
9. BackwardHopMultiplier: spec says all directions equal at 0.15s, code adds 1.5x backward penalty
10. Lane direction: spec says Row1=LEFT, formula in test gives Row1=RIGHT (odd=right)

## Integration Test Gaps (historic, most now addressed)
- No Frog+Hazard collision in live world -- PARTIALLY ADDRESSED by seam tests
- No Frog+ScoreSubsystem cross-system test -- ADDRESSED in IntegrationTest.cpp
- No GameMode+ScoreSubsystem communication test -- ADDRESSED in IntegrationTest.cpp
- No LaneManager actual spawning test (only config queries) -- STILL MISSING
- No input-to-movement pipeline test -- STILL MISSING
- No timer-triggers-death end-to-end test -- ADDRESSED in DelegateWiringTest.cpp
