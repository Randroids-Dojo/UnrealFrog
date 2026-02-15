# QA Lead Memory

*Persistent knowledge accumulated across sessions. First 200 lines loaded automatically.*

## Sprint 7 Plan (current)
- **Sprint 7 goal**: Play-test + tuning + quick fixes. NO new features until play-test complete.
- **Play-test is a BLOCKING dependency** -- all feature tasks blocked until play-test resolves
- **Scope**: Option (a) -- play-test + tuning + quick fixes only. Stretch goal for feel features only if play-test is clean.
- **GameCoordinator refactor**: DEFERRED to Sprint 8 (too risky to combine with 2-sprint play-test backlog)
- **Section 18 added**: Domain-expert post-implementation review. QA Lead reviews tests, Engine Architect reviews C++, Game Designer reviews gameplay.
- **Test count**: 154 passing (0 failures after boundary math fix)
- **M_FlatColor.uasset + functional-tests-in-CI**: DROPPED per Section 17. M_FlatColor moved to "packaging prerequisites" checklist.

### Sprint 7 Play-Test Checklist (QA Lead owns this)
1. Hop in all 4 directions -- responsive (< 100ms)
2. Death + respawn cycle (road hazard, river, timer)
3. Score updates on HUD match scoring subsystem
4. High score persists across game-over -> new game
5. VFX: hop dust visible, death puff visible, scale animation runs
6. Death flash: red overlay appears and fades
7. Music: title track plays, switches to gameplay on start, LOOPS
8. Timer bar visible and counts down, warning sound at < 16.7%
9. Title screen: "PRESS START" visible, high score shows if > 0
10. Wave transition: "WAVE 2!" announcement appears
11. Game over -> title -> restart cycle works

### Sprint 7 Test Priorities
- P1: Seam 14 (GameMode -> LaneManager wave difficulty) -- promote from DEFERRED to COVERED
- P1: Boundary test comments with exact arithmetic (new standard)
- P2: Input-to-movement pipeline seam test
- P2: "Visually verified" column in seam matrix

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
