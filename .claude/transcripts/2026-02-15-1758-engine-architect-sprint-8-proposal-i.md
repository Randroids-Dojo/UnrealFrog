# Conversation: 2026-02-15

| Field | Value |
|-------|-------|
| Date | 2026-02-15 |
| Session ID | `247322c9-2ef2-44f0-a3c1-77b896924763` |
| Model | claude-opus-4-6 |
| Tokens | ~3,339,480 input, ~210 output |
| Duration | 3m 18s |

---

## User (17:58 UTC)

<teammate-message teammate_id="engine-architect" color="green" summary="Sprint 8 technical proposal for 7 tasks">
## Engine Architect Sprint 8 Proposal

I have read `.team/agreements.md`, the retrospective log (all of Sprint 7 retro and Stakeholder Review post-Sprint 7), the seam matrix, and all relevant source files. Here is my technical analysis.

---

### Task 1: P0 — Fix Score Pop Positioning (~25 LOC)

**Current bug:** `FroggerHUD.cpp:116` hardcodes score pop to `FVector2D(20 + textLen*10, 10)` — always top-left corner.

**Approach:** In `DrawHUD()` where score pops are created (lines 108-123), replace the hardcoded position with a world-to-screen projection of the frog's position. Use `GetOwningPlayerController()->ProjectWorldLocationToScreen(FrogLocation, ScreenPos)` to get the frog's screen-space coordinates. Store the resulting `FVector2D` as `Pop.Position`. The frog reference can come from `UGameplayStatics::GetPlayerPawn()`.

**Files touched:** `FroggerHUD.cpp` only.
**Estimate:** ~25 LOC (add frog lookup, projection, null guards).
**Risk:** Low. Canvas coordinate space needs verification — `ProjectWorldLocationToScreen` returns 0..ViewportSize coords which should match Canvas coords directly. Need to test with the actual camera at Z=2200.
**I should drive this.** It touches HUD drawing internals that require understanding of UE Canvas vs viewport coordinate systems.

---

### Task 2: P0 — Fix VFX Visibility (~40 LOC)

**Current bug:** VFX meshes use scale 0.5→3.0 world units. At camera Z=2200, FOV 50, a 300cm sphere subtends roughly 1.5 pixels. Invisible.

**Approach:** Add a helper function `CalculateScaleForScreenSize(FVector WorldLocation, float DesiredScreenFraction)` to `FroggerVFXManager`. The math:
```
DesiredWorldSize = 2.0 * CameraDistance * tan(FOV/2) * DesiredScreenFraction
Scale = DesiredWorldSize / MeshBaseSize  (100 UU for engine primitives)
```
For FOV 50, Z=2200, a 5% screen fraction = ~192 UU world size → scale ~1.9 at spawn, ~5.7 at end.

Camera distance comes from `UGameplayStatics::GetPlayerCameraManager()`. We do NOT hardcode Z=2200 — we query it, making the system resilient to camera changes.

**Apply to all VFX calls:**
- `SpawnDeathPuff`: StartScale 0.5→2.0, EndScale 3.0→6.0, Duration 0.5s→0.8s (longer to register visually)
- `SpawnHopDust`: StartScale 0.15→0.5, EndScale 0.4→1.5 (subtle but visible)
- `SpawnHomeSlotSparkle`: StartScale 0.1→0.4, EndScale 0.3→1.0

**Files touched:** `FroggerVFXManager.h` (new helper declaration), `FroggerVFXManager.cpp` (helper implementation + scale updates).
**Estimate:** ~40 LOC.
**I should drive this.** Camera math is squarely my domain.

---

### Task 3: P0 — Fix VFX Hardcoded Positions (~20 LOC)

**Current bug:** `SpawnRoundCompleteCelebration()` (line 156-161) and `HandleHomeSlotFilled()` (line 203-209) hardcode `SlotColumns = {1, 4, 6, 8, 11}` and cell size `100.0` and row `14`.

**Approach:** Add a pointer or query path to the GameMode's `HomeSlotColumns`, `HomeSlotRow`, and `GridCellSize`. Two options:
1. Pass a `TArray<FVector>` of slot world positions to the VFXManager when slots are initialized
2. Query GameMode directly via `GetWorld()->GetAuthGameMode()`

I prefer option 2 — no new coupling, no stored state. The VFXManager already has `GetWorld()`. In `HandleHomeSlotFilled`, cast `GetWorld()->GetAuthGameMode()` to `AUnrealFrogGameMode*`, read `HomeSlotColumns`, `HomeSlotRow`, and `GridCellSize`, compute position from those. Same pattern for `SpawnRoundCompleteCelebration`.

**Files touched:** `FroggerVFXManager.cpp` only.
**Estimate:** ~20 LOC.
**I should drive this.** Trivial change, no design debate needed.

---

### Task 4: P0 — PlayUnreal C++ Hooks (Navigate, DevOps Drives)

Per the Stakeholder Review consensus, DevOps drives PlayUnreal with me navigating. The architecture is PythonScriptPlugin for the Python client layer + C++ hooks for input injection and state query.

**C++ hooks needed from the engine side:**

1. **Input injection:** `AFrogCharacter::RequestHop(FVector Direction)` is already public and BlueprintCallable. The Python script can call it directly via PythonScriptPlugin using `unreal.FrogCharacter.request_hop()`. No new C++ code needed for basic hop injection.

2. **State query:** We need a simple query function. I propose adding a `UFUNCTION(BlueprintCallable)` to GameMode:
```cpp
FString GetGameStateJSON() const;
```
Returns `{"score":N, "lives":N, "wave":N, "frogPos":[x,y], "gameState":"Playing"}`. ~30 LOC. This avoids building a full HTTP server — PythonScriptPlugin can call it directly.

3. **Game reset:** `StartGame()` and `ReturnToTitle()` are already public BlueprintCallable.

4. **Screenshot:** `FScreenshotRequest` / `UGameplayStatics::TakeScreenshot` — this is engine API, no custom code needed.

**My contribution as navigator:**
- Review DevOps's Python client for correctness of UE API calls
- Verify input injection works in PIE mode (not just standalone)
- Provide the `GetGameStateJSON()` function (~30 LOC)
- Ensure PythonScriptPlugin can access game objects via `unreal.find()` or `unreal.get_editor_subsystem()`

**Files touched:** `UnrealFrogGameMode.h` (1 new function declaration), `UnrealFrogGameMode.cpp` (implementation).
**Estimate from me:** ~30 LOC. DevOps owns the Python side (~100-150 LOC).
**I navigate, DevOps drives.** The heavy lifting is Python scripting, not engine code.

---

### Task 5: P1 — Temporal Passability Assertion (~30 LOC)

**The formula:** `HopDuration < MinGapCells * GridCellSize / (MaxBaseSpeed * MaxSpeedMultiplier)`

**Where the values live:**
- `HopDuration` → `AFrogCharacter::HopDuration` (0.15s)
- `MinGapCells` → `FLaneConfig::MinGapCells` (min across all configs = 2 cells)
- `GridCellSize` → `AFrogCharacter::GridCellSize` (100 UU) or `ALaneManager::GridCellSize`
- `MaxBaseSpeed` → max of all `FLaneConfig::Speed` values in `SetupDefaultLaneConfigs()` (250 UU/s for Motorcycle)
- `MaxSpeedMultiplier` → `AUnrealFrogGameMode::MaxSpeedMultiplier` (2.0)

**Current margin check:** 
```
HopDuration = 0.15s
MinGap = 2 cells * 100 UU = 200 UU
MaxSpeed = 250 * 2.0 = 500 UU/s
Time to cross gap = 200 / 500 = 0.4s
Margin = 0.4 - 0.15 = 0.25s  (OK)
```

But the tightest real case is different — the frog hops INTO a gap that is closing. The frog needs to complete a hop (0.15s) before the trailing hazard arrives. The gap travel time must exceed `HopDuration`.

**Approach:** Add a SIMPLE_AUTOMATION_TEST that:
1. Creates a GameMode, reads all lane configs
2. For each config, computes `GapTime = MinGapCells * GridCellSize / (Config.Speed * GM->MaxSpeedMultiplier)`
3. Asserts `FrogCharacter::HopDuration < GapTime` with margin
4. Reports the tightest lane

**Files touched:** New test in existing `Tests/` directory (e.g., `PassabilityTest.cpp`).
**Estimate:** ~30 LOC.
**I should drive this.** It is a physics/timing analysis that is core engine architect territory.

---

### Task 6: P1 — UFroggerGameCoordinator Extraction

**Analysis of current GameMode responsibilities (12+):**
1. Scene setup (spawn LaneManager, GroundBuilder, Camera, Light)
2. Frog delegate wiring
3. Audio delegate wiring
4. VFX delegate wiring
5. Score delegate wiring
6. State machine (SetState, transitions)
7. Timer management (TickTimer)
8. Home slot management (TryFillHomeSlot, ResetHomeSlots)
9. Difficulty scaling (GetSpeedMultiplier, GetGapReduction)
10. Hop orchestration (HandleHopCompleted)
11. Death orchestration (HandleFrogDied)
12. Wave completion (OnWaveComplete, OnRoundCompleteFinished)

**What to extract:** Items 2-5 (all delegate wiring) into `UFroggerGameCoordinator : UGameInstanceSubsystem`. This is the cleanest cut — wiring is purely setup code in BeginPlay, has no runtime dependencies, and the coordinator just calls existing public APIs on existing objects. The GameMode's BeginPlay would shrink from 127 lines to ~30 (spawn actors + call `Coordinator->WireAll()`).

**My recommendation: DEFER to Sprint 9.** Reason:
- Sprint 8 already has 4 P0 tasks + 2 P1 tasks
- The coordinator refactor is purely internal — zero player-visible benefit
- It touches the most critical file in the codebase (GameMode) during a sprint where we are also adding PlayUnreal hooks to that same file
- Per agreement §4, we should not have two unrelated changes in the same file in the same sprint
- The risk of merge conflicts and broken wiring outweighs the cleanup benefit this sprint

If we do it this sprint, it blocks all other GameMode work (Tasks 4, 7) because of the one-writer-per-file rule.

---

### Task 7: P1 — Gap Reduction Implementation (~40 LOC)

**Current state:** `ApplyWaveDifficulty()` scales speed but does NOT reduce gaps. `GetGapReduction()` computes `(Wave-1) / WavesPerGapReduction` but the result is never consumed by `ApplyWaveDifficulty`.

**Approach:** Store the original `MinGapCells` as a `BaseMinGapCells` on each hazard (same pattern as `BaseSpeed`). In `ApplyWaveDifficulty`, compute `AdjustedGap = max(1, BaseMinGapCells - GapReduction)` and reposition hazards in each lane.

The repositioning is the tricky part. Currently hazards move continuously and wrap around. "Reducing gaps" means spawning more hazards or tightening spacing. I prefer **spawning more hazards** — it is more true to the arcade original and avoids repositioning live actors mid-game.

But spawning mid-wave has its own problems (where to spawn? off-screen?). Simpler alternative: **tighten the wrap positions** so hazards spend less time off-screen, effectively increasing visible density. This requires no new actor spawning.

**My recommendation:** Implement the "tighten wrap" approach:
1. Add `float WrapPadding` to `AHazardBase` (default = GridCellSize)
2. In `ApplyWaveDifficulty`, reduce `WrapPadding` by `GapReduction * GridCellSize * 0.5`
3. Hazards wrap sooner, appearing more frequently in the play field

**Files touched:** `HazardBase.h` (new UPROPERTY), `HazardBase.cpp` (use WrapPadding in WrapPosition), `LaneManager.cpp` (apply in ApplyWaveDifficulty).
**Estimate:** ~40 LOC across 3 files.
**I should drive this.** Wrap mechanics and physics timing are core engine work.

---

### Dependencies

```
Task 1 (Score Pop)     — independent
Task 2 (VFX Vis)       — independent  
Task 3 (VFX Positions) — independent (same files as 2, so do sequentially)
Task 4 (PlayUnreal)    — independent (adds to GameMode, sequential with 7)
Task 5 (Passability)   — independent
Task 6 (Coordinator)   — BLOCKS 4 and 7 if done this sprint (same file)
Task 7 (Gap Reduction) — after Task 4 (both touch GameMode)
```

### Driver Assignments

| Task | Driver | Navigator |
|------|--------|-----------|
| 1. Score Pop | Engine Architect | QA Lead (verify position) |
| 2. VFX Visibility | Engine Architect | Game Designer (verify feel) |
| 3. VFX Positions | Engine Architect | (trivial, self-review OK) |
| 4. PlayUnreal C++ | DevOps | Engine Architect |
| 5. Passability | Engine Architect | Game Designer (tuning implications) |
| 6. Coordinator | DEFER | — |
| 7. Gap Reduction | Engine Architect | QA Lead (verify difficulty seam) |

### Sprint Scope Opinion

**Do:** All 4 P0s + Tasks 5 and 7 from P1. That is 6 tasks.
**Defer:** Task 6 (GameCoordinator). It conflicts with Tasks 4 and 7 on the same file, has zero player-visible benefit, and the sprint already has 6 tasks. The earliest safe sprint for this refactor is Sprint 9 when PlayUnreal stabilizes and no new GameMode hooks are being added.

Total estimated LOC from my hand: ~185 LOC (25+40+20+30+30+40).

### Risks

1. **ProjectWorldLocationToScreen in headless/NullRHI:** This API may return zeros without a real viewport. Score pop tests will need to verify position logic without Canvas. Mitigate: test the math separately, visual verify via PlayUnreal screenshot.

2. **PythonScriptPlugin PIE access:** We assume PythonScriptPlugin can access game objects during PIE (Play-In-Editor). If it can only access editor objects, we may need the HTTP server fallback. DevOps should spike this first.

3. **Gap reduction rebalancing:** Tightening wraps changes difficulty feel. Game Designer needs to validate that Wave 3+ is harder but not impossible. The passability test (Task 5) will serve as a safety net.

4. **One-writer-per-file on GameMode:** Tasks 4 and 7 both touch `UnrealFrogGameMode.cpp`. I propose: Task 4 adds `GetGameStateJSON()` first, then Task 7 adds gap reduction wiring. Sequential, not parallel. I drive both sections of GameMode changes in a single commit to avoid conflicts.

</teammate-message>

## Assistant (17:58 UTC)

Engine Architect's input arrived after the plan was written. Let me evaluate what needs to change in the plan and whether there are any disagreements to resolve.

Key differences from the current plan:

1. **Engine Architect proposes adding `GetGameStateJSON()` to GameMode** (~30 LOC) instead of a separate `UPlayUnrealHelpers` Blueprint Function Library. DevOps proposed the helpers class. These are competing approaches -- I need to note this.

2. **Engine Architect assumes PythonScriptPlugin** for PlayUnreal, not Remote Control API. DevOps pivoted to Remote Control API. Engine Architect hasn't seen that pivot (message lag, Section 1).

3. **Engine Architect proposes Gap Reduction as P1** (~40 LOC). My plan deferred this to Sprint 9. Worth reconsidering since it's only 40 LOC and the temporal passability test (Task 3 in my plan) would catch any safety issues.

4. **Engine Architect agrees: defer GameCoordinator to Sprint 9.** Aligned with my plan.

5. **LOC estimates differ slightly:** Score pop 25 vs my 15, VFX vis 40 vs my 30. Engine Architect's estimates are from the actual implementer, so they should be authoritative.

- **Used SendMessage**

- **Used SendMessage**

## Assistant (17:59 UTC)

Now let me update the plan file to incorporate Engine Architect's input.

- **Used Read** on `Docs/Planning/sprint8-plan.md`

## Assistant (17:59 UTC)

Now let me apply the Engine Architect's updates.

- **Used Edit** on `Docs/Planning/sprint8-plan.md`

- **Used Edit** on `Docs/Planning/sprint8-plan.md`

- **Used Edit** on `Docs/Planning/sprint8-plan.md`

- **Used Edit** on `Docs/Planning/sprint8-plan.md`

## Assistant (17:59 UTC)

Now update the LOC estimates for VFX/HUD tasks to match Engine Architect's numbers.

- **Used Edit** on `Docs/Planning/sprint8-plan.md`

- **Used Edit** on `Docs/Planning/sprint8-plan.md`

- **Used Edit** on `Docs/Planning/sprint8-plan.md`

- **Used Edit** on `Docs/Planning/sprint8-plan.md`

## Assistant (18:00 UTC)

Now add Gap Reduction as a stretch task in Phase 3, and update the deferred items table.

- **Used Edit** on `Docs/Planning/sprint8-plan.md`

## Assistant (18:00 UTC)

Now update the deferred items table and driver assignment summary.

- **Used Edit** on `Docs/Planning/sprint8-plan.md`

- **Used Edit** on `Docs/Planning/sprint8-plan.md`

## Assistant (18:00 UTC)

Now add a risk for ProjectWorldLocationToScreen in headless mode (Engine Architect flagged this).

- **Used Edit** on `Docs/Planning/sprint8-plan.md`

## Assistant (18:00 UTC)

Now update the dependency graph to reflect the changes.

- **Used Edit** on `Docs/Planning/sprint8-plan.md`

## Assistant (18:01 UTC)

Good. The plan is updated. Let me send a final confirmation to team-lead.

- **Used SendMessage**

## Assistant (18:01 UTC)

The Sprint 8 IPM is complete. The final plan is at `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Docs/Planning/sprint8-plan.md`, updated to incorporate all 5 agents' input:

**Sprint 8: PlayUnreal Automation + Visual/Perception Fixes**
- 19 tasks across 5 phases
- 5 P0s: PlayUnreal (Remote Control API + Python client), 3 VFX/HUD fixes, lock file, temporal passability
- 5 P1s: 3 difficulty perception signals (audio pitch, wave fanfare, ground color), visual smoke test, seam matrix
- 1 stretch: Gap reduction via wrap tightening
- 1 P2: Section 17 cleanup
- Target: 7+ per-subsystem commits, 168-171 tests
- Contingency plan if Remote Control API spike fails
- All cross-domain reviewers assigned per Section 18
