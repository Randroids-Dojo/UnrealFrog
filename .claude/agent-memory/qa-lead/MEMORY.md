# QA Lead Memory

*Persistent knowledge accumulated across sessions. First 200 lines loaded automatically.*

## Sprint 1 State (as of Sprint 2 IPM)
- 5 test files, 38 total tests (all unit-level, no integration tests)
- Test files: ScoreSubsystemTest, GameStateTest, FrogCharacterTest, LaneSystemTest, CollisionSystemTest
- All tests use `NewObject<>()` isolation -- no live UWorld tests exist
- No map, no visuals, no input wiring, no HUD -- nothing playable

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

## Integration Test Gaps Identified
- No Frog+Hazard collision in live world
- No Frog+ScoreSubsystem cross-system test
- No GameMode+ScoreSubsystem communication test
- No LaneManager actual spawning test (only config queries)
- No input-to-movement pipeline test
- No timer-triggers-death end-to-end test

## Key File Locations
- Spec: /Docs/Design/sprint1-gameplay-spec.md
- Headers: /Source/UnrealFrog/Public/Core/ (FrogCharacter.h, ScoreSubsystem.h, UnrealFrogGameMode.h, HazardBase.h, LaneManager.h, LaneTypes.h)
- Tests: /Source/UnrealFrog/Tests/ (5 files)
- Agreements: /.team/agreements.md
- Retro log: /.team/retrospective-log.md
