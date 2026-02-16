# Sprint 11 Plan — River Feel + Constants Sync

*Date: 2026-02-16*
*Facilitator: XP Coach*
*Participants: XP Coach, Engine Architect, Game Designer, QA Lead, DevOps Engineer*
*Sprint Goal: Make the river crossing feel right. Eliminate cross-boundary constant duplication.*

## Context

Sprint 10's collision mismatch bug was caused by independently-maintained constants in C++ and Python with no shared source of truth. The frog's platform detection margin was tightened in C++ but Python's path planner still used the old value, causing reliable deaths on river logs. All 4 agents agree: the structural fix is a single source of truth via `GetGameConfigJSON()`, and the feel fix is making `PlatformLandingMargin` a tunable UPROPERTY.

Secondary theme: VFX have been invisible for 7+ sprints despite correct positioning (confirmed by spatial tests). The root cause is likely material/rendering, not spawn position. Investigation is P1.

## Section 17 Resolutions

**UFroggerGameCoordinator extraction (4th deferral): DROPPED.**
All 4 agents agree. GameMode is ~908 LOC — large but manageable. The real complexity reductions already happened via subsystem extractions (ScoreSubsystem, VFXManager, AudioManager) in Sprints 3-5. UE's GameMode IS the coordinator by design. Revisit threshold: if GameMode exceeds 1200 LOC or a second game mode is needed.

**Audio quality bar (5th carry-forward): DROPPED.**
All 4 agents agree. 8-bit procedural audio fits the arcade aesthetic and proving-ground mission. The architecture supports drop-in replacement if higher fidelity is ever needed. Five carry-forwards = the team has spoken.

## P0 Tasks (Must Do)

### Task 1: Baseline build verification (DevOps)
Build Game + Editor, run all tests, confirm 183 pass / 0 fail before any changes.
**Acceptance criteria:** Both UBT targets succeed, `run-tests.sh --all` shows 183 passed / 0 failed.
**Files:** None (verification only)
**Effort:** 15 min

### Task 2: GetGameConfigJSON() UFUNCTION (Engine Architect)
Single source of truth for all game constants. Reads live UPROPERTY values from GameMode + FrogCharacter at runtime (not class defaults). Returns JSON with: `cellSize`, `capsuleRadius`, `gridCols`, `hopDuration`, `platformLandingMargin`, `gridRowCount`, `homeRow`. Also writes `Saved/game_constants.json` as side effect for offline tooling use.
**Acceptance criteria:** UFUNCTION callable via RC API, returns valid JSON, test verifies all fields present and non-zero.
**Files:** `UnrealFrogGameMode.h`, `UnrealFrogGameMode.cpp`, `Tests/GameConfigTest.cpp` (or extend PlayUnrealTest.cpp)
**Blocked by:** Task 1
**Effort:** 45 min (implement + test + build)
**Commits:** 1

### Task 3: PlatformLandingMargin UPROPERTY (Engine Architect)
Extract hardcoded capsule radius subtraction in `FindPlatformAtCurrentPosition()` into `UPROPERTY(EditAnywhere)` with default 17.0f (`FrogRadius * 0.5`). Update `FindPlatformAtCurrentPosition()` to use the new property instead of `GetScaledCapsuleRadius()`. Include in `GetGameConfigJSON()` output.
**Acceptance criteria:** `PlatformLandingMargin` is readable from FrogCharacter, `FindPlatformAtCurrentPosition` uses it, default produces 83% landing zone on 2-cell log.
**Files:** `FrogCharacter.h`, `FrogCharacter.cpp`
**Blocked by:** Task 1, Task 2 (for JSON inclusion)
**Cross-domain review:** Game Designer confirms default value, QA Lead confirms testability
**Effort:** 15 min
**Commits:** 1 (can combine with Task 2 if tightly coupled)

### Task 4: FindPlatform.EdgeDistance regression tests (QA Lead)
8-10 parameterized tests exercising `FindPlatformAtCurrentPosition()` boundary conditions in a UWorld. Tests read `PlatformLandingMargin` from the frog object at runtime (tuning-resilient per Section 2). Categories: `UnrealFrog.Collision.FindPlatform.*`

Test cases:
1. Dead center on platform -> FOUND
2. Comfortable interior (25% offset) -> FOUND
3. Just inside margin (EffectiveHalfWidth - 1.0) -> FOUND
4. Just outside margin (EffectiveHalfWidth + 1.0) -> NOT FOUND
5. At raw platform edge (HalfWidth) -> NOT FOUND
6. Wrong row (Y off by 1 cell) -> NOT FOUND
7. Non-rideable hazard -> NOT FOUND
8. Two platforms, frog closer to first -> FOUND (first)

**Acceptance criteria:** All tests pass, `run-tests.sh --collision` runs them.
**Files:** `Tests/CollisionTest.cpp` (new file)
**Blocked by:** Task 3 (needs PlatformLandingMargin UPROPERTY)
**Cross-domain review:** Engine Architect reviews test correctness
**Effort:** 1 session
**Commits:** 1

### Task 5: Remove hardcoded constants from Python (DevOps)
Replace hardcoded `CELL_SIZE`, `GRID_COLS`, `HOP_DURATION`, `FROG_CAPSULE_RADIUS`, `PLATFORM_INSET` in 3 files with values from `GetGameConfigJSON()`. Add `get_config()` method to `client.py` that calls the UFUNCTION and caches the result. Write `Tools/PlayUnreal/game_constants.json` fallback for offline use. Update `build-and-verify.sh` to export constants after build.

Python-only constants that stay: `SAFETY_MARGIN`, `POST_HOP_WAIT`, `API_LATENCY` (agent tuning, no C++ equivalent).

**Acceptance criteria:** Zero hardcoded game constants in `path_planner.py`, `debug_navigation.py`, `test_crossing.py`. `game_constants.json` exists and is current.
**Files:** `client.py`, `path_planner.py`, `debug_navigation.py`, `test_crossing.py`, `build-and-verify.sh`
**Blocked by:** Task 2 (needs GetGameConfigJSON UFUNCTION)
**Cross-domain review:** Engine Architect confirms JSON field names match
**Effort:** 1 hour
**Commits:** 1

### Task 6: Seam matrix update (QA Lead)
Add 2 new seam entries:
- `FindPlatformAtCurrentPosition + FinishHop` (the seam that broke in Sprint 10)
- `PathPlanner (Python) + FindPlatformAtCurrentPosition (C++)` (cross-boundary seam, verified via GetGameConfigJSON sync)
**Files:** `Docs/Testing/seam-matrix.md`
**Effort:** 15 min
**Commits:** 1 (can combine with Task 4)

### Task 7: run-tests.sh category filters (DevOps)
Add `--collision` and `--gameplay` flags to `run-tests.sh` mapping to `UnrealFrog.Collision.*` and `UnrealFrog.Gameplay.*`. Add retro-notes reminder nudge at end of test run output.
**Files:** `Tools/PlayUnreal/run-tests.sh`
**Blocked by:** Task 4 (collision tests must exist to verify filter)
**Effort:** 15 min
**Commits:** 1 (can combine with Task 5)

## P1 Tasks

### Task 8: VFX material/rendering investigation (Engine Architect)
Investigate why FlatColorMaterial VFX actors are invisible in `-game` mode despite correct positions (confirmed by spatial tests). Hypotheses: (a) shaders not compiled for runtime materials in game mode, (b) emissive not wired in FlatColorMaterial, (c) rendering layer/visibility issue. Take PlayUnreal screenshots to diagnose. Fix if root cause is identified.
**Files:** `FlatColorMaterial.h/.cpp`, `FroggerVFXManager.cpp` (likely)
**Acceptance criteria:** Either fix committed with screenshot evidence, or root cause documented for Sprint 12.
**Cross-domain review:** QA Lead verifies via PlayUnreal screenshot
**Effort:** 30-60 min (investigation) + 5-50 LOC (fix)
**Commits:** 0-1

### Task 9: [Gameplay] multi-step UWorld tests (QA Lead)
6 scenario tests simulating hop sequences with actual position-based survival/death. Categories: `UnrealFrog.Gameplay.*`

Scenarios:
1. Hop forward through safe zone -> survive
2. Hop into road hazard -> die
3. Hop onto river platform -> survive
4. Hop into river without platform -> die
5. Fill all 5 home slots -> wave increments (regression for Sprint 9 wave bug)
6. Multi-hop road crossing with moving hazards -> position-based survival

**Files:** `Tests/GameplayTest.cpp` (new file)
**Blocked by:** None (independent)
**Effort:** 1 session
**Commits:** 1

### Task 10: Wave logic investigation (Engine Architect or QA Lead)
Sprint 9 QA found filling 5 home slots didn't trigger wave increment. Reproduce via PlayUnreal, identify root cause (likely RoundComplete state transition), fix if possible.
**Acceptance criteria:** Wave increments after filling 5 slots, or root cause documented.
**Effort:** 30 min investigation
**Commits:** 0-1

### Task 11: SYNC annotation convention (DevOps)
Add `// SYNC: <file>:<constant>` comments to remaining cross-boundary constants. Voluntary `--check-sync` flag on run-tests.sh (warns but does not block). Low priority since GetGameConfigJSON eliminates most duplication.
**Files:** Affected source files + `run-tests.sh`
**Effort:** 30 min
**Commits:** 1

## P2 Tasks (Stretch/Defer)

### Task 12: Score pop / hop dust visibility tuning (Game Designer spec + Engine Architect impl)
If Task 8 (VFX investigation) finds and fixes the rendering issue:
- Score pops: 1.0s minimum duration, 18pt minimum font size
- Hop dust: increase from 3% to 5% screen size, 0.3s to 0.5s duration
**Blocked by:** Task 8 (must fix VFX rendering first)
**Effort:** 30 min
**Commits:** 1

### Task 13: Visual re-verification of Sprint 8 changes (QA Lead)
Run PlayUnreal verify_visuals.py to confirm score pops, home celebrations, ground color, wave fanfare are visible.
**Blocked by:** Task 8 (VFX rendering must be fixed first)
**Effort:** 1 session

### Task 14: Keep-alive editor + Live Coding research (DevOps)
Carried forward, 2nd deferral. Drop deadline: Sprint 13. All agents agree: defer.

### Task 15: Declarative scene description research
Carried forward, 2nd deferral. Drop deadline: Sprint 13. All agents agree: defer.

## Dependency Graph

```
Task 1 (baseline build)
  ├── Task 2 (GetGameConfigJSON) ──► Task 3 (PlatformLandingMargin)
  │     │                                 └── Task 4 (FindPlatform tests)
  │     │                                       └── Task 7 (run-tests.sh filters)
  │     └── Task 5 (Python constants sync)
  │           └── Task 7 (run-tests.sh filters)
  └── Task 8 (VFX investigation) ──► Task 12 (VFX tuning) ──► Task 13 (visual re-verify)

Independent: Task 6 (seam matrix), Task 9 ([Gameplay] tests), Task 10 (wave investigation), Task 11 (SYNC annotations)
```

## Execution Plan

**Phase 0** (Lead): Write sprint plan (this document)
**Phase 1** (Sequential — DevOps):
  - Task 1: Baseline build verification
**Phase 2** (Parallel — Engine Architect + QA Lead + DevOps):
  - Engine Architect: Task 2 (GetGameConfigJSON) -> Task 3 (PlatformLandingMargin)
  - QA Lead: Task 9 ([Gameplay] tests) + Task 6 (seam matrix) — independent, can start immediately
  - DevOps: Prepare Python-side config refactor (offline, using fallback JSON)
**Phase 3** (Sequential then parallel):
  - QA Lead: Task 4 (FindPlatform tests) — after Task 3 lands
  - DevOps: Task 5 (Python constants sync) — after Task 2 lands
  - Engine Architect: Task 8 (VFX investigation) — after Tasks 2+3 committed
**Phase 4** (Parallel):
  - DevOps: Task 7 (run-tests.sh filters) + Task 11 (SYNC annotations)
  - QA Lead: Task 10 (wave logic investigation)
**Phase 5** (If time permits):
  - Task 12 (VFX tuning) + Task 13 (visual re-verification)
**Phase 6** (Lead): Retrospective

## Team

| Agent | Tasks | Role |
|-------|-------|------|
| team-lead (xp-coach) | Plan, retro, process enforcement | Coordinator, challenger |
| engine-architect | 2, 3, 8, (12) | C++ UFUNCTIONs, VFX investigation |
| qa-lead | 4, 6, 9, 10, (13) | Collision tests, gameplay tests, seam matrix |
| devops-engineer | 1, 5, 7, 11 | Build baseline, Python sync, tooling |
| game-designer | Review Task 3 default | Cross-domain review for feel parameters |

## Agreements Changes (to apply during retrospective)

### NEW Section 24: Cross-Boundary Constant Synchronization
Any game constant that appears in both C++ and Python MUST have a single source of truth. C++ exposes constants via `GetGameConfigJSON()` UFUNCTION. Python reads them at startup via RC API. Hardcoded numeric constants that duplicate C++ values are prohibited in `Tools/PlayUnreal/`. Fallback: `Tools/PlayUnreal/game_constants.json` updated by `build-and-verify.sh`.

### UPDATE Section 18: Add Cross-Boundary Review Requirement
Any change to collision geometry, grid constants, or platform detection logic REQUIRES review from DevOps (catches Python tooling impact) AND Game Designer (catches feel/spec violations).

### NEW Section 25: Retro Notes Living Document
Agents record observations in `.team/retro-notes.md` as things happen. Post-commit nudge after `Source/` or `Tools/PlayUnreal/` changes.

## Definition of Done (Section 5a)

1. Build succeeds — Game + Editor
2. All tests pass — existing 183 + new collision + gameplay tests (target: ~200)
3. Seam tests exist for FindPlatform + FinishHop and PathPlanner + FindPlatform seams
4. Visual verification — if VFX fix lands (Task 8), screenshots proving VFX visible
5. QA sign-off — QA Lead confirms game is playable, river crossing feels fair at 83% zone
6. Project left in working state

## Success Criteria

Sprint 11 is successful when:
- `GetGameConfigJSON()` returns all game constants and Python reads from it (zero hardcoded duplicates)
- `PlatformLandingMargin` is a tunable UPROPERTY at 17.0f default (83% landing zone)
- `run-tests.sh --collision` runs 8+ boundary tests for `FindPlatformAtCurrentPosition`
- `run-tests.sh --gameplay` runs 6+ multi-step scenario tests
- Both Section 17 overdue items are formally dropped in retrospective log
- Estimated commit count: 6-8 (P0) + 2-4 (P1)

## Open Questions Resolved

| Question | Resolution | Domain Expert |
|----------|-----------|---------------|
| FrogRadius*0.5 (83%) right feel? | Yes. Classic Frogger benchmark validates. Ship it, A/B test in Sprint 12. | Game Designer |
| test_crossing.py mandatory? | No. Unit test is per-commit gate. Integration test at sprint QA. | QA Lead + DevOps consensus |
| Audio quality bar? | DROPPED per Section 17. 8-bit procedural fits the mission. | All 4 agents |
| GameCoordinator extraction? | DROPPED per Section 17. GameMode is manageable at 908 LOC. | All 4 agents |
