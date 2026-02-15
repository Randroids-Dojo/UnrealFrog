# XP Coach Memory

*Persistent knowledge accumulated across sessions. First 200 lines loaded automatically.*

## Sprint 6 Retrospective Input (2026-02-14)

### Key Findings
- Sprint 6 delivered 5 commits for 4 subsystems -- commit granularity agreement (Section 4) worked
- Section 17 exercised: M_FlatColor.uasset and functional-tests-in-CI DROPPED with rationale
- TickVFX dead code (Sprint 5 P0) was fixed immediately -- good tech debt discipline
- QA play-test still incomplete after 2 sprints -- most serious process failure
- Section 1 renamed from "Mob Programming" to "Driver Model" -- formalizing reality
- Defect escape rate 0.65% but unreliable (no gameplay verification)

### Agreements Proposed for Sprint 7
- QA tag required in commit messages ("QA: verified" or "QA: blocked")
- Post-implementation code review by second agent before retro (replaces navigator review)
- Sprint scope limit: 2-3 independent deliverables max
- Apply Section 17 to P2 carry-forwards (visual regression, packaging CI, rename PlayUnreal)

### Process Pattern Discovered
Changes that the driver can self-enforce (commit granularity, test coverage) stick.
Changes that require a second agent to act (QA play-test) do NOT stick.
Bottleneck is coordination, not discipline.

### Sprint 7 P0 (Non-Negotiable)
Full gameplay play-test covering everything from Sprint 5 forward BEFORE any new features.

### Chronic P2 Carry-Forwards (Sprint 7 Section 17 candidates)
- Visual regression testing (carried since Sprint 3)
- Packaging step in CI (carried since Sprint 3)
- Rename PlayUnreal E2E to "Scenario" (carried since Sprint 4)

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

### Trend Data
| Metric | S1 | S2 | S3 | S4 | S5 | S6 |
|--------|----|----|----|----|----|----|
| Tests | 38 | 81 | 101 | 123 | 148 | 154 |
| Commits | - | 16 | ~6 | 1 | 1 | 5 |
| Defect escape | - | 5% | 2% | 1.6% | unknown | 0.65%* |
| Agents active | 3 | 3 | 3 | 3 | 2 | 3 |

*Sprint 6 defect rate is test-only bugs, no gameplay verification performed.

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

### Sprint 1 Stats
- 38 unit tests across 5 test files
- 7 headers, 6 implementations
- Systems: FrogCharacter, ScoreSubsystem, UnrealFrogGameMode, LaneManager, HazardBase
