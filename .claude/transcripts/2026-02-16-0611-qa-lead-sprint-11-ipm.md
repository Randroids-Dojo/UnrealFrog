# Conversation: 2026-02-16

| Field | Value |
|-------|-------|
| Date | 2026-02-16 |
| Session ID | `486a1fce-113f-4722-9497-667c8c9ccddd` |
| Model | claude-opus-4-6 |
| Tokens | ~55,126 input, ~1 output |
| Duration | 3s |

---

## User (06:11 UTC)

<teammate-message teammate_id="qa-lead" color="yellow" summary="QA Lead Sprint 11 IPM input — all 5 areas">
## QA Lead — Sprint 11 IPM Input

I already sent detailed analysis to the team lead. Here is the same input structured for your facilitation questions.

---

### 1. My Top Priorities for Sprint 11

| Priority | Item | Effort | Dependencies |
|----------|------|--------|-------------|
| **P0** | `[Collision.FindPlatform.EdgeDistance]` regression tests (8-10 tests) | 1 session | Needs PlatformLandingMargin UPROPERTY from Engine Architect first, so tests read the tunable value |
| **P0** | Seam matrix update (2 new seams for FindPlatform + Python/C++ boundary) | 15 min | None |
| **P1** | `[Gameplay]` multi-step UWorld tests (6 scenarios) | 1 session | None — can proceed in parallel |
| **P1** | Visual re-verification via PlayUnreal | 1 session | Depends on VFX material investigation — need Engine Architect to look at why FlatColorMaterial doesn't render on VFX actors in -game mode |
| **P1** | test_crossing.py as collision commit gate (process proposal) | Discussion item | None |

---

### 2. Perspective on Retro 10 Action Items

**P0: GetGameConfigJSON() UFUNCTION** — AGREE. Single source of truth is the structural fix. My FindPlatform regression test will read PlatformLandingMargin from the frog object at test time (tuning-resilient per Section 2), but the Python side absolutely needs this.

**P0: Remove hardcoded constants from path_planner.py** — AGREE. The Sprint 10 collision mismatch proves this is not optional.

**P0: [Collision.FindPlatform.EdgeDistance] regression test** — AGREE, this is mine. Detailed design below.

**P0: PlatformLandingMargin as UPROPERTY** — AGREE. My regression test depends on this existing as a readable property. I recommend Engine Architect lands this first so I can write tests against the actual value rather than hardcoding 34.0f.

**P1: SYNC annotation convention** — AGREE but LOW priority for me. The GetGameConfigJSON approach makes manual annotations less critical.

**P1: Post-commit retro-notes reminder** — AGREE with DevOps's "light nudge" approach. Not a hard gate.

**P1: [Gameplay] test category** — AGREE, this is mine. 6 scenarios filling the gap between isolation tests (2s headless) and PlayUnreal E2E (120s live editor).

---

### 3. New Items Not in Retro Action List

**3a. VFX material rendering investigation (P1).**
Death puff, score pops, hop dust, and home celebration VFX are all NOT VISIBLE after 3 QA runs with 22 screenshots. Spatial tests prove position is correct. The root cause is likely material/rendering — FlatColorMaterial may not render on dynamically spawned AActor meshes in `-game` mode. This needs Engine Architect investigation before I can do visual re-verification.

**3b. Wave logic bug (P1).**
In my Sprint 9 QA runs, filling 5 home slots did not trigger wave increment. `homeSlotsFilledCount` went to 5 but `wave` stayed at 1. This may be a RoundComplete state transition issue. Needs investigation.

**3c. Seam matrix gaps (P0).**
Two new seams needed:
- `FindPlatformAtCurrentPosition + FinishHop` — the seam that broke in Sprint 10
- `PathPlanner (Python) + FindPlatformAtCurrentPosition (C++)` — cross-boundary seam, verified via GetGameConfigJSON sync

---

### 4. Section 17 Overdue Items

**(a) UFroggerGameCoordinator extraction (4th deferral):** **DROP.** GameMode is large but functional. Extracting a coordinator is a refactoring exercise with zero player value and high risk of introducing regressions. The project's mission is proving agentic UE dev, not architectural purity.

**(b) Audio quality bar (5th carry-forward):** **DROP.** 8-bit procedural audio is appropriate for the project's purpose. We are not shipping to consumers. The 5 sprints of indecision prove nobody on the team considers this important enough to do — that is the answer.

---

### 5. Open Questions — My Positions

**FrogRadius * 0.5 (83% landing zone) — right feel?**
I support Game Designer's recommendation. Classic Frogger uses ~80% of visual width as the landing zone. 83% is close to that benchmark. The key insight: this should be tunable via PlayUnreal A/B testing (set margin to 0.3, 0.5, 0.7 of FrogRadius, run test_crossing.py with each, measure death rate). But for Sprint 11, 0.5 is a reasonable default — ship it, measure later.

**test_crossing.py mandatory for collision commits?**
**Yes, with a tiered approach:**
1. Fast gate (always): `[Collision.FindPlatform.*]` unit tests in headless mode (~2s)
2. Integration gate (collision changes only): `test_crossing.py` against live editor (~130s total)

Add a `--collision` flag to `run-tests.sh` that runs both. Any change to `FindPlatformAtCurrentPosition`, `HandleHazardOverlap`, capsule radius, or platform width constants triggers the integration gate. The 130s cost is justified — Sprint 10's bug took 4 minutes to fix but a full retrospective to analyze. Prevention is cheaper than post-mortem.

**Effort estimates:**
- FindPlatform regression tests: 1 focused session (depends on PlatformLandingMargin UPROPERTY landing first)
- [Gameplay] tests: 1 focused session (independent, can start immediately)
- Visual re-verification: 1 session (blocked on VFX material investigation)
- Seam matrix update: 15 minutes (can do anytime)
- test_crossing.py gate process change: Discussion only, no code from me

---

### FindPlatform Test Design Detail

`FindPlatformAtCurrentPosition()` at `FrogCharacter.cpp:398-436` uses `TActorIterator`, so tests MUST create a UWorld via `SpatialTestHelper::CreateTestWorld()`. NewObject tests will early-return at line 402 and always pass vacuously.

**Boundary cases (8-10 tests):**

| # | Case | Frog X offset from platform center | Expected |
|---|------|-------------------------------------|----------|
| 1 | Dead center | 0 | FOUND |
| 2 | Comfortable interior | HalfWidth * 0.25 | FOUND |
| 3 | Just inside margin | EffectiveHalfWidth - 1.0 | FOUND |
| 4 | Just outside margin | EffectiveHalfWidth + 1.0 | NOT FOUND |
| 5 | At raw platform edge | HalfWidth | NOT FOUND |
| 6 | Wrong row (Y off by 1 cell) | 0 | NOT FOUND |
| 7 | Submerged platform | 0 | NOT FOUND |
| 8 | Non-rideable hazard | 0 | NOT FOUND |
| 9 | Two platforms, frog on first | Between two, closer to first | FOUND (first) |

Category: `UnrealFrog.Collision.FindPlatform.*`

Each case is a separate IMPLEMENT_SIMPLE_AUTOMATION_TEST (our established pattern for run-tests.sh category filtering).

</teammate-message>

## Assistant (06:11 UTC)

That's 3 of 4. QA Lead's input is thorough -- detailed test design for FindPlatform, agrees on both Section 17 drops, wants tiered collision testing. All 3 agents so far are aligned on the P0s and on dropping both overdue items. Waiting for game-designer.
