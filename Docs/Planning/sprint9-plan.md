# Sprint 9 Plan — Close the Feedback Loop

*Date: 2026-02-15*
*Sprint Goal: The agent can see what it builds.*

## Context

Strategic Retrospective 9 identified the root cause of 7 sprints of unverified visual output: the feedback loop between writing code and seeing the result is open. Web dev closes this loop automatically (write → save → see: 0.1s). UE dev leaves it open (write → compile → launch → look: 180s, optional).

Sprint 9 closes the loop with three deliverables:
1. **Auto-screenshot build gate** — `build-and-verify.sh` makes visual verification automatic
2. **Spatial position tests** — `[Spatial]` test category catches "actor at wrong position" bugs
3. **Visual verification of Sprint 8 debt** — Actually look at every visual change from Sprint 8

## P0 Tasks (Must Do)

### Task 1: build-and-verify.sh (DevOps)
Auto-screenshot build gate per §23. UBT build → tests → launch game → screenshot → print path.
File: `Tools/PlayUnreal/build-and-verify.sh`

### Task 2: --spatial filter in run-tests.sh (DevOps)
Add `--spatial` flag mapping to `UnrealFrog.Spatial` test path.
File: `Tools/PlayUnreal/run-tests.sh`

### Task 3: [Spatial] position assertion tests (Engine Architect)
5+ tests verifying actor positions after spawn: VFX, frog, camera, hazards, home slots.
File: `Source/UnrealFrog/Tests/SpatialTest.cpp`
Blocked by: Task 2, Task 8

### Task 8: Baseline build verification (DevOps)
Build Game + Editor, run all tests, confirm 170 pass / 0 fail before any changes.

### Task 10: Visual verification of Sprint 8 changes (QA Lead)
Run the game, verify all Sprint 8 visual changes, take screenshots.
Blocked by: Task 1, Task 7

## P1 Tasks

### Task 4: GetObjectPath / GetPawnPath UFUNCTIONs (Engine Architect)
Deterministic RC API object paths. Eliminates 13-candidate brute-force.
Files: GameMode .h/.cpp + test

### Task 5: Strengthen acceptance_test.py (DevOps)
Score > 0 (not >= 0). Death-only runs flagged.
File: `Tools/PlayUnreal/acceptance_test.py`

### Task 6: State-diff tracking in client.py (DevOps)
`get_state_diff()` returns current + delta from previous call.
File: `Tools/PlayUnreal/client.py`

### Task 7: Expand verify_visuals.py (QA Lead)
Cover all VFX/HUD: score pops, home slot celebrations, wave transitions, ground color.
File: `Tools/PlayUnreal/verify_visuals.py`

## Dependency Graph

```
Task 8 (baseline build)
  ├── Task 1 (build-and-verify.sh)
  │     └── Task 10 (visual verification) ──► Task 11 (retrospective)
  ├── Task 3 (spatial tests) ◄── Task 2 (--spatial filter)
  │     └── Task 11 (retrospective)
  └── Task 4 (deterministic paths)

Independent: Task 2, 5, 6, 7, 9
```

## Execution Plan

**Phase 0** (Lead): Write sprint plan (this document)
**Phase 1** (Parallel — DevOps + Engine Architect):
  - DevOps: Task 8 (build baseline) → Task 1 (build-and-verify.sh) + Task 2 (--spatial)
  - Engine Architect: Task 4 (deterministic paths) — starts after Task 8
**Phase 2** (Parallel — all agents):
  - Engine Architect: Task 3 (spatial tests) — after Tasks 2, 8
  - DevOps: Task 5 (acceptance_test) + Task 6 (state-diff)
  - QA Lead: Task 7 (expand verify_visuals.py)
**Phase 3** (Sequential — QA Lead):
  - Task 10: Visual verification of Sprint 8 changes
**Phase 4** (Lead): Task 11 — Retrospective

## Team

| Agent | Tasks | Role |
|-------|-------|------|
| team-lead (xp-coach) | 9, 11 | Coordinator, process enforcement, challenger |
| devops-engineer | 8, 1, 2, 5, 6 | Build infra, PlayUnreal tooling |
| engine-architect | 3, 4 | C++ spatial tests, GameMode UFUNCTIONs |
| qa-lead | 7, 10 | Visual verification, quality gate |

## Definition of Done (§5a)

1. Build succeeds — Game + Editor
2. All tests pass — existing 170 + new spatial tests
3. Seam tests exist for new interactions (spatial tests cover spawning seams)
4. Visual verification — screenshots of all Sprint 8 visual changes in `Saved/Screenshots/`
5. QA sign-off — QA Lead confirms game is playable and visual effects are visible
6. Project left in working state

## Success Criteria

Sprint 9 is successful when:
- `build-and-verify.sh` runs end-to-end and produces a screenshot
- `run-tests.sh --spatial` runs 5+ position assertion tests
- Screenshots exist proving score pops, death VFX, home celebrations, ground color, and wave fanfare are all visible
- The team has verified visual output for the first time as a standard process step
