# Sprint 7 Plan: Consolidation — Play-Test, Tune, Fix

*Created: Sprint 6 Full-Team Retrospective (all 5 agents participated)*
*Sprint theme: "Verify what we have, improve what the player sees"*

## Goal

Close two sprints of unverified gameplay. Play-test everything from Sprint 5+6, tune the core loop, fix bugs found. No new mechanics, no refactors. Feel features are stretch goals only if play-test is clean.

**Decided by vote (3-2):** QA Lead, XP Coach, DevOps voted for strict consolidation. Engine Architect and Game Designer wanted 1-2 feel features included. Compromise: feel features are stretch goals, not commitments.

## Phase 0: Quick Fixes (blocks play-test)

| Task | Owner | Est. | Priority |
|------|-------|------|----------|
| Fix SaveHighScore per-tick writes — persist only at game over / title | Engine Architect | 5 min | P0 |
| Fix duplicate wave-complete detection in HandleHopCompleted vs TryFillHomeSlot | Engine Architect | 15 min | P0 |
| Cache VFXManager and AudioManager pointers in BeginPlay | Engine Architect | 10 min | P1 |
| Add pre-flight stale process cleanup to run-tests.sh | DevOps | 10 min | P1 |

## Phase 1: Full Gameplay Play-Test (blocks all feature work)

**Owner:** QA Lead + Game Designer
**Method:** Launch game via play-game.sh, play 5 sessions through Wave 3+

### 11-Point Verification Checklist (QA Lead)
1. [ ] Hop in all 4 directions — responsive, < 100ms feel
2. [ ] Death + respawn cycle — road hazard, river, timer
3. [ ] Score updates on HUD match scoring subsystem
4. [ ] High score persists across game-over -> new game
5. [ ] VFX: hop dust visible, death puff visible, scale animation runs
6. [ ] Death flash: red overlay appears and fades (~0.3s)
7. [ ] Music: title track plays, switches to gameplay on start, loops
8. [ ] Timer bar visible, counts down, warning sound at < 16.7%
9. [ ] Title screen: "PRESS START" visible, high score shows if > 0
10. [ ] Wave transition: "WAVE 2!" announcement appears
11. [ ] Game over -> title -> restart cycle works

### Tuning Pass (Game Designer)
6 key numbers to evaluate during play-test:
- `HopDuration` (currently 0.15s) — too slow? too fast?
- `HopArcHeight` — does the arc feel satisfying?
- `InputBufferWindow` (currently 0.1s) — responsive enough at Wave 3+ speed?
- `TimePerLevel` (currently 30.0s) — too generous? too tight?
- `DifficultySpeedIncrement` (currently 0.1 per wave) — ramp too gradual?
- `WavesPerGapReduction` (currently every 2 waves) — does difficulty feel progressive?

### Bug Budget
Sprint 2 precedent: 4-5 bugs found during first play-test. Budget time to fix.

## Phase 2: Seam Test Promotion

| Task | Owner | Priority |
|------|-------|----------|
| Seam 14 test: GameMode -> LaneManager wave difficulty | QA Lead | P1 |

## Phase 3: Stretch Goals (only if play-test is clean)

| Task | Owner | Priority |
|------|-------|----------|
| Multiplier visibility on HUD ("x3" next to score) | Game Designer + Engine Architect | Stretch |
| Death freeze frame (0.05s pause) + screen shake (0.2s) | Game Designer + Engine Architect | Stretch |
| Screenshot capture in play-game.sh (--screenshot flag) | DevOps | P2 |

## Process Changes for Sprint 7

1. **Section 18 (NEW):** Post-implementation domain expert review before commit. Unanimous vote.
2. **QA tag in commits:** Every sprint commit must include "QA: verified by [agent]" or "QA: blocked — [reason]".
3. **WIP limit:** 2-3 deliverables max per sprint. Consolidation sprint = play-test + fixes + at most 1 stretch feature.
4. **Retros require all agents.** Sprint 6 was the first full-team retro. Continue this.

## Sprint 8 Preview (not committed, for context)

Items deferred to Sprint 8 by unanimous vote:
- UFroggerGameCoordinator extraction (delegate wiring refactor)
- §17 deadline for remaining P2 carry-forwards (visual regression, packaging, E2E rename)
- Potential gameplay features (turtle submerge, lane variety, new game modes)
