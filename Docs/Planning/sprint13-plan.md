# Sprint 13: Fix What's Broken, Verify What Works

**Date:** 2026-02-17
**Goal:** Every visual system must be proven working in the running game. Zero "code exists but unverified" items remain after this sprint.

## Context

Sprint 12 closed the visual feedback loop (38 screenshots, 3 bugs found). Sprint 13 uses that loop to fix the remaining game-breaking issues:

1. **VFX invisible for 8 sprints** — code correct, never visually confirmed working
2. **Wave completion suspected broken** — Sprint 9 QA reported 5 home slots didn't trigger wave 2
3. **Acceptance test never run end-to-end** — acceptance_test.py exists but was never executed

## P0 Deliverables

### P0-1: VFX Visibility Fix
**Owner:** Engine Architect (driver), QA Lead (navigator)

VFX has been "code-verified" but invisible since Sprint 5. Root cause hypothesis: VFX actors spawn at Z=0, same as ground plane — Z-fighting makes them invisible from top-down camera at Z=2200.

Tasks:
1. QA runs `verify_visuals.py` against current build to confirm invisibility
2. Engine writes test: VFX actor spawns at Z > ground plane Z
3. Engine fixes SpawnVFXActor to elevate VFX above ground (e.g., Z+100)
4. Rebuild + QA re-runs verify_visuals.py to confirm visibility
5. Screenshot evidence committed per §21

### P0-2: Wave Completion Verification
**Owner:** Engine Architect (driver), Game Designer (navigator)

Sprint 9 QA reported: filling 5 home slots didn't trigger wave increment.

Tasks:
1. Engine writes test: fill all 5 home slots → verify `CurrentWave` increments to 2
2. If test fails → fix state transition logic
3. If test passes → investigate via PlayUnreal (wave transition may work in code but not in live game)
4. QA verifies via PlayUnreal: fill 5 slots, observe wave counter change

### P0-3: End-to-End Acceptance Test
**Owner:** QA Lead (driver), Engine Architect (navigator)

acceptance_test.py uses blind hopping. Update to use path planner for reliable navigation.

Tasks:
1. Update acceptance_test.py to use `navigate_to_home_slot()` from path_planner
2. Run against live game with invincibility OFF
3. Full pass = frog navigates to home slot, game state reflects it
4. Record gameplay video as demo evidence

## P1 Deliverables

### P1-1: Sprint 12 Regression Tests
- Test: RidingZOffset applied correctly on river rows
- Test: VFX Z-position above ground plane
- Test: Wave completion state machine (5 fills → RoundComplete → wave++)

### P1-2: Demo Recording
- 30-60s gameplay video showing: multi-part models, frog crossing road/river, VFX, wave completion
- Saved to Saved/Screenshots/demo/

## Team

| Agent | Role | Tasks |
|-------|------|-------|
| xp-coach | Team Lead / Process | Coordination, task assignment, retro prep |
| engine-architect | C++ Systems | VFX fix, wave test, regression tests |
| qa-lead | Testing / Verification | PlayUnreal runs, visual verification, acceptance test |

## Build Order

1. Build Game + Editor (must pass before any PlayUnreal work)
2. Run `run-tests.sh --all` (212 tests must pass)
3. Engine writes new tests + fixes
4. Rebuild
5. QA runs PlayUnreal scripts
6. Visual evidence committed
7. Retro

## Success Criteria

- [ ] VFX visible in screenshots from running game
- [ ] Wave counter increments from 1→2 after 5 home slots filled
- [ ] acceptance_test.py achieves FULL PASS (home slot reached)
- [ ] Demo video recorded
- [ ] All tests pass (target: 220+ with new tests)
