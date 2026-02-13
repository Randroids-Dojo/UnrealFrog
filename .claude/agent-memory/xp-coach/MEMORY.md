# XP Coach Memory

*Persistent knowledge accumulated across sessions. First 200 lines loaded automatically.*

## Sprint 2 Planning (2026-02-12)

### Sprint Goal
"Make UnrealFrog playable from the editor."

### Plan Location
`/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Docs/Planning/sprint2-plan.md`

### Key Decisions
- Phase 0 (spec alignment) must complete before any integration work
- 10 spec-vs-implementation mismatches identified and documented
- Engine Architect is primary driver for most C++ tasks
- Map creation (Task 9) requires the Unreal Editor -- may need human assistance
- Enhanced Input assets also require editor -- C++ fallback planned
- 13 tasks across 4 phases (0-3)
- 18-item QA acceptance checklist defined

### Critical Path
Phase 0 -> Task 3 (meshes) -> Task 6 (collision) -> Task 7 (orchestration) -> Task 8 (HUD) -> Task 9 (map) -> Task 12 (play-test)

### Current State Machine Mismatch (Phase 0, Item 1)
- Current: EGameState has 4 states (Menu, Playing, Paused, GameOver)
- Needed: 7 states (Title, Spawning, Playing, Paused, Dying, RoundComplete, GameOver)
- This is the biggest Phase 0 change -- touches every system

### Score Subsystem Mismatches (Phase 0, Items 2-5)
- MultiplierIncrement: 0.5 -> 1.0
- MaxLives: 5 -> 9
- Time bonus: percentage-based -> RemainingSeconds * 10
- Missing: AddBonusPoints(), AddHomeSlotScore()

### Sprint 1 Stats
- 38 unit tests across 5 test files
- 7 headers, 6 implementations
- Systems: FrogCharacter, ScoreSubsystem, UnrealFrogGameMode, LaneManager, HazardBase

### Open Questions
- Can agents launch Unreal Editor to create maps?
- Legacy input fallback vs Enhanced Input only?
- Paused state: Sprint 2 or Sprint 3?
