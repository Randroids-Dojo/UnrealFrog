# Sprint 2 Plan: Make UnrealFrog Playable from the Editor

*Drafted by: XP Coach*
*Date: 2026-02-12*
*Status: ACTIVE*

---

## Sprint Goal

**"Make UnrealFrog playable from the editor."**

Press Play in the Unreal Editor and experience a complete Frogger game loop: a frog on a 13x15 grid, arrow-key hopping, cars that kill you, logs that carry you, score/lives/timer on the HUD, game over when lives reach zero, and fillable home slots at the top.

---

## Definition of Done

A Sprint 2 deliverable is NOT complete until ALL of the following are true:

- [ ] Project builds successfully (Game + Editor targets via UBT)
- [ ] All automated tests pass (38 existing + new Sprint 2 tests)
- [ ] Press Play in the editor and the game is fully interactive
- [ ] QA Lead has verified gameplay via the 18-item acceptance checklist
- [ ] The project is left in a working, playable state for Sprint 3

---

## What Is Playable After Sprint 2

When you press Play in the Unreal Editor:

- You see a colored grid with 15 rows: green start zone, gray road lanes with colored boxes (cars) moving across, green median, blue river lanes with brown cylinders (logs) floating by, and amber/green goal zone with 5 home slots
- A bright green sphere (the frog) sits at center-bottom
- Arrow keys hop the frog one cell at a time with a visible arc animation
- Red boxes (cars) crossing your path kill you -- you see the frog disappear, lose a life on the HUD, and respawn at start after 1 second
- Brown cylinders (logs) in the river carry you -- land on one and ride it across; miss and you die
- Top of screen shows score, high score, and a timer bar draining from green to red
- Bottom shows frog-icon lives and wave number
- Reach a home slot at the top: it fills, you score 200 points, frog respawns
- Fill all 5: round complete, 1000 bonus, wave 2 begins faster
- Lose all lives: "GAME OVER" appears center screen

---

## Phase 0: Spec Alignment

**MUST complete before any integration work. All 10 items are blocking.**

The Sprint 1 C++ implementation diverged from the gameplay spec in 10 places. These must be resolved first so that Phase 1-3 work builds on a correct foundation.

**Driver: Engine Architect** (owns the C++ source + tests)

| # | File(s) Affected | Current Value | Spec Value | Change Required |
|---|-------------------|---------------|------------|-----------------|
| 1 | `UnrealFrogGameMode.h` | `EGameState` has 4 states: `Menu, Playing, Paused, GameOver` | 7 states: `Title, Spawning, Playing, Paused, Dying, RoundComplete, GameOver` | Add `Title`, `Spawning`, `Dying`, `RoundComplete`. Rename `Menu` to `Title`. Keep `Paused`. Update `SetState()` callers, `StartGame()`, and state transition logic to support timed transitions (Spawning 1.0s, Dying 0.5s, RoundComplete 2.0s). |
| 2 | `ScoreSubsystem.h` | `MultiplierIncrement = 0.5f` | Integer increments: multiplier goes 1, 2, 3, 4, 5 | Change to `1.0f`. Update `AddForwardHopScore()` and all tests that assert on multiplier progression. |
| 3 | `ScoreSubsystem.h` | `MaxLives = 5` | `MaxLives = 9` (classic arcade display ceiling) | Change default to `9`. Update any test asserting on max lives. |
| 4 | `ScoreSubsystem.cpp` | `AddTimeBonus()` uses percentage: `(Remaining / Max) * 1000` | `RemainingSeconds * 10` (integer truncation) | Change signature to `AddTimeBonus(float RemainingSeconds)`. Formula: `Score += FMath::FloorToInt(RemainingSeconds) * 10`. Update all callers and tests. |
| 5 | `ScoreSubsystem.h/.cpp` | No round completion bonus | 1000 points on round complete | Add `AddBonusPoints(int32 Points)` method. GameMode calls `AddBonusPoints(1000)` when all 5 homes filled. Add `HomeSlotPoints = 200` as UPROPERTY with `AddHomeSlotScore()` method. |
| 6 | `UnrealFrogGameMode.h` | `WavesPerGapReduction = 3` | `WavesPerGapReduction = 2` (spec: gaps reduce every 2 waves) | Change default to `2`. Update `GetGapReduction()` tests. |
| 7 | `UnrealFrogGameMode.cpp` | `GetSpeedMultiplier()` has no cap | Speed cap at 2.0x (Wave 11+) | Add `FMath::Min(2.0f, ...)` to `GetSpeedMultiplier()`. Add `MaxSpeedMultiplier = 2.0f` as UPROPERTY. Add test for cap enforcement. |
| 8 | `FrogCharacter.cpp` `GridToWorld()` | Origin at `(0,0)` maps to world `(-600, 0, 0)` | Spec agrees: `GridToWorld(0,0) = (-600, 0, 0)` | **No code change.** The implementation is correct. Update spec Appendix if any wording is ambiguous. Verify round-trip: `WorldToGrid(GridToWorld(col, row)) == (col, row)` for all valid positions. |
| 9 | `FrogCharacter.h` | `BackwardHopMultiplier = 1.5f` | All directions same speed: `HopDuration = 0.15s` | Change default to `1.0f`. Remove or deprecate the multiplier logic in `GetHopDurationForDirection()` if it scales by this value. Update tests. |
| 10 | `LaneManager.cpp` | Lane direction alternation formula | Spec: odd road rows LEFT, even road rows RIGHT. River: odd rows RIGHT, even rows LEFT. | Verify `bMovesRight` assignment in `SetupDefaultLanes()` or equivalent. Fix if alternation is inverted. Add explicit tests for direction per row. |

### Phase 0 Deliverables

- All 38 existing tests updated to match corrected values
- All 38 tests pass
- Both build targets succeed
- No functional regressions (same logic, corrected parameters)

### Phase 0 Estimated Effort

Moderate. Primarily parameter changes and test updates. Item 1 (state machine expansion) is the largest change since it requires adding timed state transitions.

---

## Phase 1: Foundation

**Can be parallelized across non-overlapping files.** Depends on Phase 0 completion.

### Task 1: Camera System

**Driver: Engine Architect** | **Navigator: Level Designer**

Create a fixed top-down camera that shows the full 13x15 grid.

**Requirements:**
- New class: `AFroggerCameraActor` (or configure the default `APlayerStart` camera)
- Fixed position above the grid center, looking down
- Pitch: -72 degrees (slightly angled, not pure top-down, for depth perception)
- FOV: 60 degrees
- Must show all 15 rows with some padding on each side
- Camera position: approximately `(0, 700, 1800)` -- tune to fit full grid
- No camera movement or follow logic in Sprint 2 (fixed camera)

**Files:**
- `Source/UnrealFrog/Public/Core/FroggerCameraActor.h` (new)
- `Source/UnrealFrog/Private/Core/FroggerCameraActor.cpp` (new)

**Tests:**
- Camera position is at expected coordinates after BeginPlay
- Camera rotation pitch is approximately -72 degrees
- All 4 grid corners are within the camera frustum

---

### Task 2: Enhanced Input System

**Driver: Engine Architect** | **Navigator: Game Designer**

Wire up player input using UE5 Enhanced Input so the frog responds to arrow keys and WASD.

**Requirements:**
- New class: `AFrogPlayerController` (extends `APlayerController`)
- Create `UInputAction` assets: `IA_HopUp`, `IA_HopDown`, `IA_HopLeft`, `IA_HopRight`, `IA_Start`, `IA_Pause`
- Create `UInputMappingContext`: `IMC_Frogger` binding arrows + WASD to hop actions, Enter/Space to Start, Escape to Pause
- Controller possesses `AFrogCharacter` and routes input actions to `RequestHop(Direction)`
- Input is blocked during `Dying`, `Spawning`, `RoundComplete` states (enforced by checking `EGameState`)
- Start input transitions from `Title` to `Spawning`, from `GameOver` to `Title`

**Files:**
- `Source/UnrealFrog/Public/Core/FrogPlayerController.h` (new)
- `Source/UnrealFrog/Private/Core/FrogPlayerController.cpp` (new)
- Enhanced Input assets: `Content/Input/IA_HopUp.uasset`, etc. (must be created in-editor or via Python scripting)

**Risk:** Enhanced Input assets (InputAction, InputMappingContext) are binary `.uasset` files that cannot be created from C++ alone. They require either the editor or Python editor scripting. The Engine Architect should create default bindings in C++ code with `UInputAction::NewObject()` as a fallback, OR create assets via the editor manually.

**Tests:**
- `RequestHop(FVector(0,1,0))` is called when Up arrow is pressed (mock or functional test)
- Input during `Dying` state produces no hop
- Start input from `Title` state triggers `Spawning` transition

---

### Task 3: Placeholder Meshes

**Driver: Engine Architect** | **Navigator: Art Director**

Attach engine-primitive meshes to all gameplay actors so they are visible when playing.

**Requirements:**
- `AFrogCharacter`: bright green sphere (radius ~40 UU)
- `AHazardBase` road hazards: red/orange boxes scaled to match hazard width
  - Car: 1-cell red box
  - Truck: 2-cell dark red box
  - Bus: 2-cell orange box
  - Motorcycle: 1-cell small yellow box
- `AHazardBase` river objects: brown cylinders scaled to match platform width
  - Small Log: 2-cell brown cylinder
  - Large Log: 4-cell brown cylinder
  - Turtle Group: 3-cell dark green cylinder
- Colors via dynamic material instances (set in BeginPlay using `UMaterialInstanceDynamic`)
- No external mesh assets needed -- use `Engine/BasicShapes/` references (`/Engine/BasicShapes/Sphere`, `/Engine/BasicShapes/Cube`, `/Engine/BasicShapes/Cylinder`)

**Files:**
- `FrogCharacter.cpp` (modify constructor or BeginPlay)
- `HazardBase.h/.cpp` (add mesh selection logic based on `EHazardType`)

**Art Director validates:**
- Color choices provide clear visual hierarchy (danger = warm colors, safe = cool/green, river = blue/brown)
- Scale is correct relative to grid cell size (100 UU)
- Frog is visually distinct from all hazards

**Tests:**
- `MeshComponent` is non-null after construction
- Mesh scale matches hazard width (e.g., truck mesh X-scale = 2.0)

---

### Task 4: Ground Plane Geometry

**Driver: Level Designer** | **Navigator: Art Director**

Build the 15-row colored ground plane so the grid is visible.

**Requirements:**
- 15 flat box meshes (thin planes), one per row, spanning the full grid width (1300 UU)
- Each row 100 UU deep, 10 UU thick
- Color scheme (via dynamic material instances):

| Row(s) | Zone | Color |
|--------|------|-------|
| 0 | Start (safe) | Bright green `(0.2, 0.8, 0.2)` |
| 1-5 | Road | Dark gray `(0.3, 0.3, 0.3)` |
| 6 | Median (safe) | Bright green `(0.2, 0.8, 0.2)` |
| 7-12 | River | Blue `(0.1, 0.3, 0.8)` |
| 13 | Goal (lower) | Amber `(0.8, 0.6, 0.1)` |
| 14 | Goal (upper) | Dark green `(0.1, 0.5, 0.1)` with 5 lighter spots for home slots |

- Home slot indicators: 5 small bright-green planes at columns 1, 4, 6, 8, 11 on row 14
- New class: `AGroundBuilder` actor that spawns all geometry in BeginPlay
- No dependencies on other gameplay systems -- purely visual

**Files:**
- `Source/UnrealFrog/Public/Core/GroundBuilder.h` (new)
- `Source/UnrealFrog/Private/Core/GroundBuilder.cpp` (new)

**Tests:**
- `AGroundBuilder` spawns exactly 15 row planes
- Row 0 plane is at correct world position `(0, 0, -5)` (centered, half-thickness below ground)
- Home slot indicators exist at the 5 expected column positions

---

### Task 5: Build.cs Module Updates

**Driver: DevOps Engineer** | **Navigator: Engine Architect**

Update the build configuration to support new Sprint 2 dependencies.

**Requirements:**
- Add `UMG` to PublicDependencyModuleNames (for HUD widgets in Task 8)
- Add `Slate`, `SlateCore` to PrivateDependencyModuleNames (UMG dependency)
- Guard `FunctionalTesting` module dependency behind `#if WITH_DEV_AUTOMATION_TESTS` or add it only to Editor target
- Verify both Game and Editor targets build with the new dependencies

**Files:**
- `Source/UnrealFrog/UnrealFrog.Build.cs` (modify)

**Current Build.cs dependencies:**
```csharp
PublicDependencyModuleNames: Core, CoreUObject, Engine, InputCore, EnhancedInput
PrivateDependencyModuleNames: (empty)
```

**Updated:**
```csharp
PublicDependencyModuleNames: Core, CoreUObject, Engine, InputCore, EnhancedInput, UMG
PrivateDependencyModuleNames: Slate, SlateCore
```

**Tests:**
- Game target builds: `Result: Succeeded`
- Editor target builds: `Result: Succeeded`

---

## Phase 2: Integration

**Sequential. Depends on all Phase 1 tasks.** This is where the isolated Sprint 1 systems become a connected game.

### Task 6: Collision Wiring

**Driver: Engine Architect** | **Navigator: QA Lead**

Connect the physics overlap system for road hazards and implement grid-based river landing checks.

**Requirements:**

**Road hazard collision:**
- `AHazardBase` road-type actors have a `UBoxComponent` collision volume
- Collision bounds are 80% of visual bounds (forgiving near-misses, per spec)
- On overlap with `AFrogCharacter`, call `Frog->Die(EDeathType::Squish)`
- Collision only active during `EGameState::Playing`

**River landing checks:**
- When a hop completes on a river row (7-12), query all `AHazardBase` actors on that row
- Check if any river object's platform bounds overlap the frog's landing cell
- Platform bounds are 110% of visual bounds (generous landing, per spec)
- If overlap: mount the frog on the platform (`CurrentPlatform = Platform`)
- If no overlap: `Frog->Die(EDeathType::Splash)`
- While riding, if frog's world X exits grid bounds: `Frog->Die(EDeathType::OffScreen)`

**Integration points:**
- `AFrogCharacter::OnHopCompleted` delegate triggers river landing check
- `AHazardBase` overlaps trigger road death
- `ALaneManager` provides the list of hazards per row for river queries

**Files:**
- `HazardBase.h/.cpp` (modify -- add collision component, overlap handler)
- `FrogCharacter.cpp` (modify -- add river landing logic in FinishHop)
- `LaneManager.h/.cpp` (modify -- add `GetHazardsOnRow(int32 Row)` query)

**Tests:**
- Frog at road hazard position triggers `Die(Squish)`
- Frog landing on river row with no platform triggers `Die(Splash)`
- Frog landing on river row overlapping a log does NOT die
- Frog carried past column 12 triggers `Die(OffScreen)`
- Collision is inactive during `Dying` state (no double-death)

---

### Task 7: Game Orchestration

**Driver: Engine Architect** | **Navigator: Game Designer**

Wire together FrogCharacter death, ScoreSubsystem, and GameMode state transitions into a complete game loop.

**Requirements:**

**Death flow:**
1. `AFrogCharacter::Die()` broadcasts `OnFrogDied`
2. `AUnrealFrogGameMode` listens, transitions to `Dying` state
3. After 0.5s timer, GameMode calls `UScoreSubsystem::LoseLife()`
4. If `Lives > 0`: transition to `Spawning`, after 1.0s call `Frog->Respawn()`, transition to `Playing`
5. If `Lives == 0`: transition to `GameOver`

**Scoring flow:**
1. `AFrogCharacter::OnHopCompleted` -- if hop was forward (row increased), call `UScoreSubsystem::AddForwardHopScore()`
2. If hop was backward (row decreased), call `UScoreSubsystem::ResetMultiplier()`
3. Sideways hops: no scoring action

**Home slot flow:**
1. When frog reaches row 14 at a home slot column: `GameMode->TryFillHomeSlot(Column)`
2. On success: `ScoreSubsystem->AddHomeSlotScore()` (200 pts), `ScoreSubsystem->ResetMultiplier()`, respawn frog
3. On failure (not a home slot column, or already filled): `Frog->Die(EDeathType::Splash)`
4. When all 5 filled: `ScoreSubsystem->AddBonusPoints(1000)`, `ScoreSubsystem->AddTimeBonus(RemainingTime)`, transition to `RoundComplete`
5. After 2.0s: increment wave, reset timer, reset home slots, transition to `Spawning`

**Timed state transitions (from Phase 0 state machine expansion):**
- `Spawning` -> `Playing`: 1.0s delay
- `Dying` -> `Spawning` or `GameOver`: 0.5s delay
- `RoundComplete` -> `Spawning`: 2.0s delay
- Use `FTimerHandle` for each transition timer

**Files:**
- `UnrealFrogGameMode.h/.cpp` (major modifications -- orchestration hub)
- `FrogCharacter.cpp` (modify -- home slot check on row 14 arrival)
- `ScoreSubsystem.h/.cpp` (modify -- new methods from Phase 0 item 5)

**Tests:**
- Death triggers state transition: `Playing` -> `Dying` -> `Spawning` -> `Playing`
- Game over triggers: `Playing` -> `Dying` -> `GameOver`
- Forward hop awards correct points with multiplier
- Home slot fill awards 200 points
- Round complete awards 1000 bonus + time bonus
- Wave increments after round complete
- Timer only counts down during `Playing`

---

### Task 8: HUD

**Driver: Engine Architect** | **Navigator: Game Designer**

Create a UMG widget overlay displaying all game state information.

**Requirements:**
- New class: `UFroggerHUDWidget` (extends `UUserWidget`)
- New class: `AFroggerHUD` (extends `AHUD`) -- creates and manages the widget

**HUD layout:**

```
+------------------------------------------+
|  SCORE: 00000    HI: 00000    TIME [====] |
|                                           |
|                                           |
|             (game area)                   |
|                                           |
|                                           |
|  LIVES: F F F    WAVE: 1                  |
+------------------------------------------+
```

**Top bar:**
- Score: left-aligned, updates via `UScoreSubsystem::OnScoreChanged`
- High Score: center, updates via `OnScoreChanged` (check `HighScore`)
- Timer Bar: right-aligned, horizontal progress bar, green->yellow->red gradient as time drains. Updates via `OnTimerUpdate`

**Bottom bar:**
- Lives: frog icons (or green circles) representing remaining lives. Updates via `OnLivesChanged`
- Wave Number: right-aligned, "WAVE 1" text. Updates on wave completion

**Special overlays:**
- "GAME OVER" text centered on screen when `EGameState::GameOver`, large white text with dark outline
- "ROUND COMPLETE" text centered during `EGameState::RoundComplete`
- "PRESS START" blinking text during `EGameState::Title`

**Font:** Use engine default font. No custom font assets in Sprint 2.

**Files:**
- `Source/UnrealFrog/Public/Core/FroggerHUDWidget.h` (new)
- `Source/UnrealFrog/Private/Core/FroggerHUDWidget.cpp` (new)
- `Source/UnrealFrog/Public/Core/FroggerHUD.h` (new)
- `Source/UnrealFrog/Private/Core/FroggerHUD.cpp` (new)

**Game Designer validates:**
- Score, lives, and timer are readable at a glance
- Timer bar color gradient conveys urgency
- Game Over text is impossible to miss
- HUD does not obscure the playing field

**Tests:**
- Widget is created and added to viewport on BeginPlay
- Score text updates when `OnScoreChanged` fires
- Lives display matches `UScoreSubsystem::Lives`
- Game Over overlay appears when state is `GameOver`

---

## Phase 3: Polish and Verification

**Depends on Phase 2 completion.** These tasks make the game actually launchable and verify everything works end-to-end.

### Task 9: Bootstrap Minimal Map via Python Script

**Driver: DevOps Engineer** | **Navigator: Engine Architect**

Create an empty `.umap` via headless Python scripting, then commit it. All gameplay content is spawned by C++ in BeginPlay — the map is just an empty container.

**Requirements:**
- Enable `PythonScriptPlugin` and `EditorScriptingUtilities` in `UnrealFrog.uproject`
- Write `Tools/CreateMap/create_frog_map.py` that creates an empty level at `/Game/Maps/FroggerMain`
- Run via: `UnrealEditor-Cmd UnrealFrog.uproject -ExecutePythonScript="Tools/CreateMap/create_frog_map.py"`
- Update `Config/DefaultEngine.ini` to set `FroggerMain` as the default editor map
- GameMode auto-spawns everything: `AGroundBuilder`, `ALaneManager`, camera, lighting
- `AFrogPlayerController` and `AFrogCharacter` set as defaults in GameMode constructor (no World Settings override needed)
- Commit the resulting `.umap` binary

**Fallback:** If Python scripting fails, test whether UE 5.7 creates a transient default world when the referenced map doesn't exist. If so, no `.umap` is needed at all.

**Files:**
- `UnrealFrog.uproject` (modify -- add plugin entries)
- `Tools/CreateMap/create_frog_map.py` (new)
- `Content/Maps/FroggerMain.umap` (new, generated)
- `Config/DefaultEngine.ini` (modify)

---

### Task 10: Integration Tests

**Driver: DevOps Engineer** | **Navigator: Engine Architect**

Write latent automation tests that verify cross-system interactions in a temporary UWorld.

**Requirements:**
- New test file: `Source/UnrealFrog/Tests/IntegrationTest.cpp`
- Tests run in automation framework (no editor required)
- Test categories under `UnrealFrog.Integration`

**Test cases:**
1. **Death-to-respawn cycle**: Create Frog + GameMode + ScoreSubsystem. Kill frog. Verify state transitions `Playing -> Dying -> Spawning -> Playing` with correct timing.
2. **Scoring integration**: Hop forward 5 times. Verify score = 10+20+30+40+50 = 150. Verify multiplier caps at 5.
3. **Home slot fill**: Place frog at row 14, home slot column. Verify slot fills, 200 points awarded, multiplier resets.
4. **Round complete**: Fill all 5 home slots. Verify 1000 bonus, time bonus, wave increment, home slots reset.
5. **Game over**: Set lives to 1. Kill frog. Verify `GameOver` state reached.
6. **Wave speed scaling**: Set wave to 11. Verify `GetSpeedMultiplier()` returns 2.0 (capped).
7. **Collision -> death -> score**: Overlap frog with road hazard. Verify death type is `Squish`, life lost, score multiplier reset.

**Files:**
- `Source/UnrealFrog/Tests/IntegrationTest.cpp` (new)

---

### Task 11: PlayUnreal Shell Script

**Driver: DevOps Engineer** | **Navigator: QA Lead**

Create a headless test runner script that launches tests without a display.

**Requirements:**
- Shell script: `Tools/PlayUnreal/run-tests.sh`
- Launches `UnrealEditor-Cmd` with `-NullRHI -NoSound -ExecCmds="Automation RunTests UnrealFrog"`
- Captures exit code and test output
- Reports pass/fail count
- Can be run from CI or command line

**Stretch goal (if editor supports it):**
- Launch the game with a specific map
- Send simulated input (via `-ExecCmds` or Remote Control API)
- Capture game state (score, lives, position) via log parsing

**Files:**
- `Tools/PlayUnreal/run-tests.sh` (new)
- `Tools/PlayUnreal/README.md` (new -- usage instructions)

---

### Task 12: QA Play-Test

**Driver: QA Lead** | **Navigator: Game Designer**

Execute the acceptance checklist against the running game. This is the final gate before Sprint 2 is marked done.

**Acceptance Checklist:**

| # | Check | Method | Pass/Fail |
|---|-------|--------|-----------|
| AC-1 | Game launches without errors when pressing Play | Editor Play-In-Editor | |
| AC-2 | Frog is visible as a green sphere at center-bottom of grid | Visual | |
| AC-3 | Arrow keys move the frog one cell per press with visible arc | Input + Visual | |
| AC-4 | Frog cannot move outside the 13x15 grid | Input at boundaries | |
| AC-5 | Road hazards (colored boxes) move across road lanes, wrapping at edges | Visual | |
| AC-6 | Collision with a road hazard kills the frog (frog disappears, life lost) | Deliberate collision | |
| AC-7 | Frog respawns at start position after death (1s delay) | Wait and observe | |
| AC-8 | River lanes show moving platforms (brown cylinders) | Visual | |
| AC-9 | Landing on a log carries the frog with the log | Hop onto log, observe | |
| AC-10 | Missing all logs on a river row kills the frog (splash death) | Hop to empty river | |
| AC-11 | Being carried off-screen by a log kills the frog | Ride log to edge | |
| AC-12 | Score increases on forward hops (visible on HUD) | Hop forward, check HUD | |
| AC-13 | Timer bar drains during gameplay | Visual | |
| AC-14 | Timer reaching 0 kills the frog | Wait for timeout | |
| AC-15 | Reaching a home slot at row 14 fills it and awards points | Navigate to home slot | |
| AC-16 | Filling all 5 home slots triggers round complete + wave 2 | Fill all 5 | |
| AC-17 | Losing all lives shows "GAME OVER" on screen | Die until 0 lives | |
| AC-18 | Wave 2+ hazards move faster than Wave 1 | Complete a round, observe | |

**Defects found during play-test are logged as issues and fixed before Sprint 2 closure.**

---

### Task 13: Audio Infrastructure Stubs

**Driver: Sound Engineer** | **Navigator: Engine Architect**

Set up the audio component and method stubs so Sprint 3 can drop in sound assets immediately.

**Requirements:**
- Add `UAudioComponent` to `AFrogCharacter`
- Create folder structure: `Content/Audio/SFX/`, `Content/Audio/Music/`
- Add empty stub methods on `AFrogCharacter`:
  - `PlayHopSound()` -- called at hop start
  - `PlayDeathSound(EDeathType)` -- called on death
  - `PlayHomeSlotSound()` -- called on home fill
- Add empty stub methods on `AUnrealFrogGameMode`:
  - `PlayRoundCompleteSound()`
  - `PlayGameOverSound()`
- Methods log to `UE_LOG` for now: `"[Audio] PlayHopSound called -- no asset assigned"`
- No actual audio assets in Sprint 2

**Files:**
- `FrogCharacter.h/.cpp` (modify -- add UAudioComponent + stubs)
- `UnrealFrogGameMode.h/.cpp` (modify -- add stubs)

**Tests:**
- `UAudioComponent` is non-null after construction
- Stub methods execute without crash (log output verified)

---

## Driver Assignments Summary

Per team agreements: domain expert drives, same agent writes test AND implementation.

| Agent | Tasks | Role |
|-------|-------|------|
| **Engine Architect** | Phase 0 (all), Tasks 1, 2, 3, 6, 7, 8 | Primary driver for all C++ systems |
| **Level Designer** | Tasks 4, 9 | Ground plane geometry, map creation |
| **Art Director** | -- | Navigator on Tasks 3, 4 (validates colors, shapes, visual hierarchy) |
| **Game Designer** | -- | Navigator on Tasks 2, 7, 8 (validates feel, flow, HUD layout) |
| **DevOps Engineer** | Tasks 5, 10, 11 | Build system, test infrastructure |
| **QA Lead** | Task 12 | Play-test execution, integration test validation |
| **Sound Engineer** | Task 13 | Audio infrastructure stubs |
| **XP Coach** | -- | Coordination, task sequencing, retrospective facilitation |

---

## Dependency Graph

```
Phase 0 (spec alignment)
  |
  v
Phase 1 (foundation) -- all tasks can run in parallel
  |
  +-- Task 1 (camera) ----------+
  +-- Task 2 (input) -----------+
  +-- Task 3 (meshes) ----------+--> Phase 2 (integration) -- sequential
  +-- Task 4 (ground planes) ---+      |
  +-- Task 5 (Build.cs) -------+      +-- Task 6 (collision wiring)
                                       |      |
                                       |      v
                                       +-- Task 7 (game orchestration)
                                       |      |
                                       |      v
                                       +-- Task 8 (HUD)
                                              |
                                              v
                                       Phase 3 (polish) -- can parallelize
                                         +-- Task 9  (map) --------+
                                         +-- Task 10 (integ tests) |
                                         +-- Task 11 (PlayUnreal)  +--> Task 12 (play-test)
                                         +-- Task 13 (audio stubs) +
```

**Critical path:** Phase 0 -> Task 3 (meshes) -> Task 6 (collision) -> Task 7 (orchestration) -> Task 8 (HUD) -> Task 9 (map) -> Task 12 (play-test)

---

## Risks and Mitigations

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| Map creation requires the editor (binary `.umap`) | Task 9 blocked if agents cannot launch editor | High | Provide a detailed checklist for manual map creation. Human stakeholder creates the map. |
| Enhanced Input assets are binary `.uasset` | Task 2 partially blocked | High | Create default bindings in C++ code. Fall back to legacy input system if needed. Alternatively use Python editor scripting. |
| First integration will surface bugs hidden by unit test isolation | Phase 2 takes longer than estimated | High | Budget extra time for Task 7. Integration tests (Task 10) catch issues early. |
| State machine expansion (Phase 0 item 1) touches every system | Cascading test failures | Medium | Do Phase 0 item 1 first, fix all tests before proceeding to other items. |
| UMG widget creation without Blueprint | HUD may be plain | Medium | Use C++-only Slate-backed UMG. Visual polish deferred to Sprint 3. |
| Collision tuning (80%/110% bounds) may feel wrong in practice | Deaths feel unfair or landings feel broken | Medium | QA Lead validates feel during play-test. Expose bounds multipliers as UPROPERTY for tuning. |

---

## Resolved Decisions (IPM)

- **Map creation**: CLI/agent-driven. C++ auto-setup (GameMode spawns everything in BeginPlay). One-time Python script bootstrap for minimal empty .umap via `UnrealEditor-Cmd -ExecutePythonScript`. No editor GUI dependency.
- **Enhanced Input assets**: Pure C++ via `NewObject<UInputAction>()` at runtime. No .uasset binary files needed.
- **Pause state**: Deferred to Sprint 3. Not priority.
- **Camera**: Standalone actor auto-spawned by GameMode, not attached to frog pawn.
- **GridToWorld convention**: Keep current (origin at 0,0), update spec documentation.

## Spike: Editor Automation Tool (Research)

**Owner: DevOps Engineer** | **Timeboxed: 2 hours**

Research and prototype a custom tool for operating the Unreal Editor programmatically from the CLI. This is a Sprint 2 spike (research only, no production deliverable) to enable richer editor interaction in Sprint 3+.

**Research questions:**
1. Can `PythonScriptPlugin` + `EditorScriptingUtilities` reliably create maps, place actors, and set World Settings via `UnrealEditor-Cmd`?
2. What editor APIs are exposed to Python? (actor spawning, property setting, asset creation, saving)
3. Can we build a reusable `Tools/UnrealEditorCLI/` script library that wraps common editor operations?
4. What are the limitations? (plugin stability, API coverage, error handling)
5. Would an MCP server (like unreal-mcp) be more capable? What does it require?
6. Is Computer Use MCP viable as a fallback for operations Python can't reach?

**Deliverable:** A research document at `Docs/Research/editor-automation-spike.md` with findings, a proof-of-concept Python script, and a recommendation for Sprint 3.

---

## Sprint 2 Exit Criteria

Sprint 2 is DONE when:

1. All Phase 0 spec mismatches are resolved
2. All 13 tasks are complete per their individual requirements
3. Both Game and Editor targets build with `Result: Succeeded`
4. All automated tests pass (existing 38 + new Sprint 2 tests)
5. QA Lead has passed all 18 acceptance checklist items
6. The game is playable end-to-end from the editor
7. Retrospective has been run and agreements updated
8. All files committed with conventional commit messages

---

## Current Codebase Reference

For context, here is what exists from Sprint 1:

| File | Purpose |
|------|---------|
| `Source/UnrealFrog/Public/Core/FrogCharacter.h` | Frog pawn: grid movement, hop arc, input buffer, death/respawn |
| `Source/UnrealFrog/Public/Core/ScoreSubsystem.h` | Scoring: points, multiplier, lives, extra lives, high score |
| `Source/UnrealFrog/Public/Core/UnrealFrogGameMode.h` | Game state machine, timer, home slots, wave progression |
| `Source/UnrealFrog/Public/Core/LaneTypes.h` | Enums: `EDeathType`, `ELaneType`, `EHazardType`; struct: `FLaneConfig` |
| `Source/UnrealFrog/Public/Core/HazardBase.h` | Base hazard actor: movement, wrapping, lane assignment |
| `Source/UnrealFrog/Public/Core/LaneManager.h` | Lane configuration and hazard spawning |
| `Source/UnrealFrog/Tests/*.cpp` | 38 unit tests across 5 test files |
| `Source/UnrealFrog/UnrealFrog.Build.cs` | Module dependencies: Core, CoreUObject, Engine, InputCore, EnhancedInput |

---

*"Optimism is an occupational hazard of programming; feedback is the treatment." -- Kent Beck*

*Sprint 2 is where feedback becomes tangible. We will see the game running for the first time.*
