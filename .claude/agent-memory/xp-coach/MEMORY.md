# XP Coach Memory

*Persistent knowledge accumulated across sessions. First 200 lines loaded automatically.*

## Sprint 5 Retrospective (2026-02-14)

### Key Findings
- Sprint 5 delivered all goals (music, VFX, HUD, title screen) but process discipline regressed
- Mob programming not practiced -- single orchestrating agent, no navigator review
- Second consecutive monolithic commit (2005 lines, 18 files) -- Sprint 4 was the same pattern
- QA play-test did not happen before commit -- Sprint 5 Definition of Done is incomplete
- Seam matrix P0 completed (17 pairs cataloged)
- Test count: 148 (0 failures, 17 categories)

### Agreements Changed in Sprint 5 Retro
- Section 1: Mandatory review gates for tasks > 200 lines
- Section 4: Per-subsystem commit rule (no monolithic sprint commits)
- Section 5 Step 9: QA play-test BEFORE commit, not after
- NEW Section 17: P1 items deferred 3x must be completed or dropped next sprint

### Chronic Deferrals (Hitting Section 17 Deadline)
- M_FlatColor.uasset: 4th deferral -- MUST resolve in Sprint 6
- Functional tests in CI: 4th deferral -- MUST resolve in Sprint 6

### Sprint 6 P0s
1. QA play-test Sprint 5 deliverables (before any new work)
2. Enforce per-subsystem commits throughout Sprint 6

### Process Trend Concern
- Sprint 2: 16 commits, phased execution, mob-like
- Sprint 3: Multiple commits, hotfix pattern
- Sprint 4: 1 commit, 2198 lines
- Sprint 5: 1 commit, 2005 lines
- Trend is toward solo-driver monolithic delivery. Need structural enforcement.

### Open Question: Mob vs Solo
The team has not done true mob programming since Sprint 1. Every sprint since has been single-driver. The new review gate rule (section 1) is a compromise -- not full mob, but not unchecked solo. Monitor whether it actually gets followed in Sprint 6.

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
