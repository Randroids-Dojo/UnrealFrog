# Conversation: 2026-02-15

| Field | Value |
|-------|-------|
| Date | 2026-02-15 |
| Session ID | `247322c9-2ef2-44f0-a3c1-77b896924763` |
| Model | claude-opus-4-6 |
| Tokens | ~890,335 input, ~57 output |
| Duration | 3m 14s |

---

## User (17:54 UTC)

<teammate-message teammate_id="qa-lead" color="orange" summary="Sprint 8 QA testing plan: 9 tests, seam matrix, risk assessment">
## Sprint 8 QA Lead Proposal

I've reviewed agreements.md, the full retrospective log, the seam matrix (19 entries), all 18 test files, and my own accumulated memory. Here is my testing and verification plan.

---

### 1. Test Plan for Sprint 8

#### New Automated Tests (8-10 tests)

**P0: Temporal Passability Assertion (1 test)**
- `FSeam_TemporalPassabilityInvariant` in SeamTest.cpp
- Reads `HopDuration`, `GridCellSize`, `MaxBaseSpeed`, `MaxSpeedMultiplier` from GM and FrogCharacter at test time (tuning-resilient per agreement section 2)
- For every road/river lane config: asserts `HopDuration < MinGapCells * GridCellSize / (Config.Speed * GM->MaxSpeedMultiplier)`
- This is the mathematical guarantee that no tuning combination creates impossible lanes. The Engine Architect flagged a 0.05s margin at max difficulty in Sprint 7 -- this test makes that an invariant.

**P0: Score Pop Position Verification (1 test)**
- `FHUD_ScorePopUsesWorldProjection` in HUDTest.cpp
- After the P0 fix lands (ProjectWorldLocationToScreen replaces hardcoded FVector2D), verify that score pop Position.X and Position.Y are derived from a projected world position, NOT hardcoded to (20 + textLen*10, 10).
- Cannot verify actual projection without a Canvas, but CAN verify the FScorePop::Position is set to something other than the old hardcoded value.

**P0: VFX Scale at Camera Distance (1-2 tests)**
- `FVFX_DeathPuffScaleForCameraDistance` in VFXTest.cpp
- After the P0 fix lands (CalculateScreenSpaceScale), verify that EndScale for death VFX is camera-distance-relative.
- Test: create VFX, call SpawnDeathPuff with a known location, check that ActiveEffects[0].EndScale produces a world-unit size that subtends > 5% of the screen at Z=2200 camera height with FOV 50.
- Math: at Z=2200, FOV 50, visible width ~= 2 * 2200 * tan(25deg) ~= 2052 UU. 5% = 103 UU. So EndScale * 100 (sphere unit size) > 103 UU, meaning EndScale > 1.03. Currently 3.0 * 100 = 300 UU world-space = 14.6% of screen -- but the sphere primitive is NOT 100 UU radius. Need to check actual mesh extent after the fix lands.

**P1: Difficulty Perception Multi-Modal (1 test)**
- `FSeam_WaveDifficultyProducesPerceptibleChange` in SeamTest.cpp
- Verify that wave N+1 speed multiplier produces >= 20% change vs wave 1 by wave 3 (the perception threshold per Game Designer's analysis). This encodes the stakeholder feedback that "no visible differences" means the ramp is below human perception.
- Read DifficultySpeedIncrement from GM; assert `(Wave3Mult - 1.0) / 1.0 >= 0.20`.

**P1: Home Slot VFX Position from Grid Config (1 test)**
- `FVFX_HomeSlotSparkleReadsGridConfig` in VFXTest.cpp
- After the P0 fix lands (remove hardcoded magic numbers from FroggerVFXManager.cpp:159), verify HandleHomeSlotFilled uses HomeSlotColumns from GM, not literals.

**P1: PlayUnreal State Query Roundtrip (1-2 tests, contingent on PlayUnreal landing)**
- `FPlayUnreal_StateQueryRoundtrip` -- reset game, hop forward 3 times, query state, verify score > 0, lives == 3, wave == 1, frogPos.Y == 3.
- `FPlayUnreal_DeathAndRecovery` -- trigger road death, verify lives == 2, state == Spawning/Playing after recovery.
- These replace the current PlayUnreal "scenario" tests that call methods directly. They should use whatever API PlayUnreal exposes (HTTP, Python bridge, etc).

**Seam matrix additions (see section 4):**
- `FSeam_PlayUnrealStateReflectsGameState` -- if PlayUnreal lands, verify the state endpoint returns accurate JSON matching GM internal state.

---

### 2. Seam Matrix Additions for Sprint 8

New seams introduced by Sprint 8 work:

| # | System A | System B | Seam Description | Priority |
|---|----------|----------|------------------|----------|
| 20 | PlayUnreal Server | GameMode | State query returns accurate score/lives/wave/state | P0 (if PlayUnreal lands) |
| 21 | PlayUnreal Server | FrogCharacter | Hop command moves frog to correct grid position | P0 (if PlayUnreal lands) |
| 22 | FroggerHUD | APlayerController | Score pop uses ProjectWorldLocationToScreen not hardcoded coords | P0 |
| 23 | FroggerVFXManager | Camera | Death VFX scale relative to camera distance, visible at Z=2200 | P0 |
| 24 | FroggerVFXManager | GameMode (HomeSlotColumns) | Home celebration reads grid config, not magic numbers | P1 |
| 25 | Difficulty scaling | Human perception | Wave 3 speed >= 20% faster than wave 1 | P1 |

Existing deferred seams to review:
- Seam 13 (GameMode -> ScoreSubsystem StartGame): Still DEFERRED. Low risk, direct call.
- Seam 15 (FrogCharacter -> FrogPlayerController input buffering): DEFERRED. Sprint 7 fixed InputBufferWindow enforcement. Should be promoted to COVERED if we add the temporal passability test (which implicitly validates the buffer works within the timing window).
- Seam 17 (GroundBuilder -> LaneManager rows): Still DEFERRED. No changes this sprint.

---

### 3. Should PlayUnreal Block All Other Work?

**No -- but it should block QA sign-off.**

Rationale:
- The VFX/HUD fixes (score pop projection, death VFX scaling, home slot positions) are ~60 LOC total and can be implemented AND unit-tested independently of PlayUnreal.
- Temporal passability and difficulty perception tests can be written now.
- **However**, until PlayUnreal exists, I cannot perform the visual verification that agreement section 9 requires. "QA: verified" requires launching the game. Without PlayUnreal, the commit message must say "QA: code-verified, visual pending."
- The VFX/HUD fixes should be done FIRST, then PlayUnreal validates them. This is the XP Coach's position and I agree with it -- build the tool, then use it to validate.

**My recommended sequencing:**
1. VFX/HUD fixes (Engine Architect) + temporal passability test (QA Lead) -- parallel, no dependencies
2. PlayUnreal core (DevOps drives, Engine Architect navigates) -- can start parallel with #1
3. QA visual verification via PlayUnreal -- blocked on #1 AND #2
4. Difficulty perception features -- blocked on PlayUnreal for validation

---

### 4. Acceptance Criteria for "VFX is Visible" (Quantitative)

These are measurable, not subjective:

**Death VFX (explosion/puff):**
- Must subtend >= 5% of visible screen width at gameplay camera distance (Z=2200, FOV 50)
- At Z=2200, FOV 50: visible width ~= 2052 UU. 5% = ~103 UU minimum diameter at full scale.
- Duration >= 0.3s (current 0.5s is fine -- but NOT shorter)
- Must be at frog's world position (verified by checking SpawnLocation == Frog->GetActorLocation())

**Score pops (+50, +100):**
- Must appear within 200 pixels of frog's projected screen position (not hardcoded to top-left)
- Font size >= 24pt (readable from couch distance)
- Duration >= 0.8s (time to read a 3-character string)
- Must float upward (DeltaY < 0 over lifetime)

**Home slot celebration:**
- Must appear at the correct home slot world position (derived from HomeSlotColumns array, not hardcoded magic numbers)
- At least 4 sparkle actors visible simultaneously
- Duration >= 1.0s

**Wave announcement text:**
- Must be centered on screen (X within 10% of center)
- Font size >= 36pt
- Duration >= 1.5s

**Validation method (once PlayUnreal exists):**
- Python script triggers each event, takes screenshot, verifies via pixel-level checks (effect at expected position, non-black pixels in expected region)
- Until then: code-level assertions on position, scale, duration values

---

### 5. Risk Assessment

**High risk:**
1. **PlayUnreal scope creep.** The full spec (HTTP server + Python client + Automation Driver input injection + screenshot capture) is 200+ LOC of new C++ infrastructure. If this takes more than 60% of sprint capacity, the VFX/HUD fixes get squeezed. **Mitigation:** Time-box PlayUnreal to 2 sessions. If it's not working by session 3, ship VFX fixes with "QA: visual pending" and carry PlayUnreal forward as P0 for Sprint 9.
2. **PythonScriptPlugin may not work in -game mode.** DevOps recommends PythonScriptPlugin but it's an editor-only plugin (Experimental). If `-game` mode can't load it, the entire approach fails. **Mitigation:** Spike-test PythonScriptPlugin availability in `-game` mode FIRST (agreement section 16). If it fails, fall back to pure C++ HTTP server approach.
3. **Camera-distance-relative VFX scaling math.** The `CalculateScreenSpaceScale` approach requires knowing the mesh's base extent. If the Engine Architect uses a 1cm base sphere vs a 100cm base sphere, the scale factor is 100x different. **Mitigation:** Define the contract in the test first (TDD): "at Z=2200, FOV 50, EndScale produces >= 103 UU diameter." Let the implementation satisfy the test.
4. **We have never successfully launched the game from an agent session.** play-game.sh exists but no agent has run it and confirmed a playable window appeared. If PlayUnreal's launch mechanism fails silently (macOS sandbox, missing display, stale processes), we are back to "QA: pending." **Mitigation:** DevOps must demonstrate a successful programmatic launch before building the API layer on top of it.
5. **One-agent-tests-at-a-time (agreement section 19) creates a bottleneck.** With 5 agents active, test queue contention will slow everyone down. The lock file mechanism is a P0 for Sprint 8 and should land in the first commit. **Mitigation:** DevOps delivers lock file mechanism as commit #1. All agents coordinate via XP Coach for test runs until then.

**Medium risk:**
6. **Temporal passability at 0.05s margin.** The test will pass (the math works), but 0.05s is a single-frame margin at 20fps. If the game ever drops below 20fps during a dense wave, the lane becomes impossible in practice even though it's passable in theory. We need PlayUnreal to verify this at runtime, not just at math level.
7. **Monolithic PlayUnreal commit.** If PlayUnreal is 200+ LOC across 3+ files, agreement section 4 requires per-subsystem commits. Ensure the C++ server, Python client, and acceptance test are separate commits.

**Low risk:**
8. VFX/HUD fixes are small, well-scoped, and have clear before/after states.
9. Temporal passability test is pure math on existing values.

---

### Summary of QA Sprint 8 Deliverables

| Priority | Deliverable | Dependencies | Test Count |
|----------|------------|-------------|------------|
| P0 | Temporal passability invariant test | None | 1 |
| P0 | Score pop position verification test | Score pop fix landed | 1 |
| P0 | Death VFX scale verification test | VFX fix landed | 1-2 |
| P0 | Visual play-test of Sprint 7 tuning | PlayUnreal OR manual launch | 0 (verification) |
| P1 | Difficulty perception threshold test | None | 1 |
| P1 | Home slot VFX position test | VFX fix landed | 1 |
| P1 | PlayUnreal state roundtrip tests | PlayUnreal landed | 1-2 |
| P1 | Visual smoke test script via PlayUnreal | PlayUnreal landed | 1 script |
| -- | Seam matrix update (6 new entries) | Varies | 0 |

**Estimated new test count: 6-9 tests (162 -> 168-171 total)**

I am ready to start on the temporal passability test and difficulty perception test immediately -- they have no dependencies on other Sprint 8 work. The VFX/HUD verification tests should be written test-first (Red) and handed to Engine Architect for implementation (Green).

</teammate-message>

## Assistant (17:54 UTC)

Strong input from QA Lead. Four agents have now responded with substantive proposals. Let me synthesize the QA Lead's input, note the disagreement between DevOps and QA on PythonScriptPlugin vs Remote Control API, and prepare to write the plan.

One important catch: QA Lead's risk #2 mentions PythonScriptPlugin concern, but DevOps already pivoted AWAY from PythonScriptPlugin TO Remote Control API in their revised proposal. The QA Lead may have been working from an earlier version of the DevOps proposal. This is exactly the message lag pattern from Section 1 -- I should not escalate, just note it.

- **Used SendMessage**

## Assistant (17:55 UTC)

Now let me proceed to write the sprint plan. I have comprehensive input from all four domain agents and the decisions are clear. Per Section 1, I should not wait indefinitely for a response that may be delayed by message lag.

- **Used Bash**: List existing sprint plans for format reference

- **Used Write** on `Docs/Planning/sprint8-plan.md`

## Assistant (17:57 UTC)

Plan written. Now let me notify the team and mark the task complete.

- **Used SendMessage**

- **Used TaskUpdate** on task 1

## Assistant (17:57 UTC)

The Sprint 8 IPM is complete. The plan is at `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Docs/Planning/sprint8-plan.md`.

**What was produced:**
- 18 tasks across 5 phases with clear dependency graph
- Every task has: priority, owner, cross-domain reviewer, files, LOC estimate, acceptance criteria
- Architecture decision: Remote Control API (spike-validated) over custom HTTP server
- Difficulty perception: 3 multi-modal signals (audio pitch, wave fanfare, ground color) with "5-second test" acceptance criterion
- Quantitative VFX acceptance criteria (>= 5% of screen width, not subjective)
- TDD workflow: QA writes Red tests first, Engine Architect writes Green implementation
- Contingency plan if the Remote Control API spike fails
- Section 17 resolution: 3 chronic P2 items dropped with rationale
- Risk register with 7 identified risks and mitigations
- Target: 7+ per-subsystem commits, 168-171 total tests
