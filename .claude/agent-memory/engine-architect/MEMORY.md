# Engine Architect Memory

*Persistent knowledge accumulated across sessions. First 200 lines loaded automatically.*

## Project Structure
- Module: UnrealFrog (primary game module)
- Public headers: Source/UnrealFrog/Public/
- Private impl: Source/UnrealFrog/Private/
- Tests: Source/UnrealFrog/Tests/
- Build.cs deps: Core, CoreUObject, Engine, InputCore, EnhancedInput

## AFrogCharacter (created session 1, updated Phase 0)
- File: Public/Core/FrogCharacter.h + Private/Core/FrogCharacter.cpp
- APawn subclass with grid-based movement
- Grid: 13 columns x 15 rows, 100 UU cell size
- Hop: 0.15s all directions (BackwardHopMultiplier=1.0, uniform), 30 UU arc height
- Input buffering during active hops
- GridPosition updated immediately on hop start (logical position = target)
- Parabolic arc: 4*alpha*(1-alpha) for Z interpolation
- Cardinal direction snapping via largest absolute axis
- Tests: 6 automation tests in Tests/FrogCharacterTest.cpp

## UScoreSubsystem (created session 2, updated Phase 0)
- File: Public/Core/ScoreSubsystem.h + Private/Core/ScoreSubsystem.cpp
- UGameInstanceSubsystem -- pure scoring logic, no gameplay dependencies
- Forward hop: PointsPerHop(10) * Multiplier, multiplier += 1.0 each consecutive hop (was 0.5)
- Multiplier capped at MaxMultiplier(5.0)
- Multiplier resets on: ResetMultiplier(), LoseLife()
- Time bonus: floor(RemainingSeconds) * 10 (was ratio-based)
- Extra life every 10,000 pts, capped at MaxLives(9, was 5), tracked via LastExtraLifeThreshold
- New methods: AddBonusPoints(int32), AddHomeSlotScore() (awards HomeSlotPoints=200)
- New tunables: MaxMultiplier(5.0f), HomeSlotPoints(200), RoundCompleteBonus(1000)
- High score persists across StartNewGame(), score/lives/multiplier reset
- Delegates: FOnScoreChanged, FOnLivesChanged, FOnExtraLife, FOnGameOver
- Tests: 13 automation tests in Tests/ScoreSubsystemTest.cpp (+MultiplierCap, BonusPoints, HomeSlotScore)
- Testing pattern: NewObject<UScoreSubsystem>() works without GameInstance for unit tests
- Note: tests directly set public UPROPERTY fields for setup (Score, Multiplier, Lives) -- valid for unit test isolation

## Lane System (created session 3)
- Types: Public/Core/LaneTypes.h -- ELaneType, EHazardType enums + FLaneConfig USTRUCT
- Hazard: Public/Core/HazardBase.h + Private/Core/HazardBase.cpp
  - AActor subclass, moves along X axis at configured Speed
  - Wrapping: teleports to opposite side when past grid boundary + hazard width buffer
  - Turtle submerge: SubmergeInterval(3.0s) -> submerge, SubmergeDuration(1.5s) -> surface
  - TickMovement() and TickSubmerge() public for unit testing
  - InitFromConfig() sets all properties from FLaneConfig
  - UBoxComponent collision, sized to hazard width
- Manager: Public/Core/LaneManager.h + Private/Core/LaneManager.cpp
  - AActor, owns TMap<int32, TArray<AHazardBase*>> HazardPool (row -> hazards)
  - SetupDefaultLaneConfigs() populates all 11 lanes per design spec
  - ValidateGaps() checks no lane has hazards >= GridColumns wide
  - CalculateSpawnPositions() evenly spaces hazards in lane
  - Road: odd rows LEFT, even rows RIGHT; River: odd rows RIGHT, even rows LEFT
  - Speeds/gaps updated to match spec Wave 1 table (Phase 0)
- Grid: 13 cols (X), rows 0-14 (Y), CellSize=100 UU
  - Row 0: Start (safe), 1-5: Road, 6: Median (safe), 7-12: River, 13-14: Goal
- Tests: 7 automation tests in Tests/LaneSystemTest.cpp

## Collision & Death System (created session 4)
- EDeathType enum added to LaneTypes.h: None, Squish, Splash, Timeout, OffScreen
- FOnFrogDied delegate updated from uint8 to EDeathType
- AFrogCharacter additions:
  - State: bIsDead, bIsGameOver, LastDeathType, CurrentPlatform (TWeakObjectPtr<AHazardBase>)
  - Tunables: RespawnDelay(1.0s), RiverRowMin(7), RiverRowMax(12)
  - Die(EDeathType): sets dead state, broadcasts OnFrogDied, schedules respawn via FTimerHandle
  - Respawn(): resets to GridPosition(6,0), clears dead state
  - IsOnRiverRow(): checks GridPosition.Y in [RiverRowMin, RiverRowMax]
  - CheckRiverDeath(): true if on river with no platform or submerged platform
  - IsOffScreen(): true if world X < 0 or >= GridColumns * CellSize
  - ShouldRespawn(): !bIsGameOver
  - UpdateRiding(DeltaTime): applies platform velocity (Speed * Direction) to frog X
  - Tick() updated: skip if dead, ride platforms on river rows, auto-detect off-screen
  - RequestHop() rejects input while dead
- Tests: 7 automation tests in Tests/CollisionSystemTest.cpp
  - Logic-focused tests using NewObject<> (no UWorld required for decision functions)
  - Dynamic delegates cannot use AddLambda -- tests verify state instead

## Game State Machine (created session 5, updated Phase 0)
- EGameState enum in UnrealFrogGameMode.h: Title, Spawning, Playing, Paused, Dying, RoundComplete, GameOver (was Menu/Playing/Paused/GameOver)
- File: Public/Core/UnrealFrogGameMode.h + Private/Core/UnrealFrogGameMode.cpp
- AUnrealFrogGameMode : AGameModeBase
- State machine: Title->Playing (StartGame), Playing->Paused (PauseGame), Paused->Playing (ResumeGame), Playing/Paused->GameOver (HandleGameOver), GameOver->Title (ReturnToTitle)
- Invalid transitions silently rejected (guard clauses)
- Level timer: TimePerLevel(30s), TickTimer(dt) counts down, paused state blocks ticking
- Home slots: 5 total, TryFillHomeSlot() fills and checks wave complete
- Wave progression: OnWaveComplete() increments CurrentWave, resets homes and timer
- Difficulty: WavesPerGapReduction=2 (was 3), DifficultySpeedIncrement=0.1, MaxSpeedMultiplier=2.0 (new cap)
- GetSpeedMultiplier() capped at MaxSpeedMultiplier
- ReturnToTitle() (renamed from ReturnToMenu)
- Delegates: FOnGameStateChanged, FOnWaveComplete, FOnTimerUpdate, FOnHomeSlotFilled, FOnTimerExpiredNative, FOnWaveCompletedNative
- Tests: 9 automation tests in Tests/GameStateTest.cpp (updated for Title rename and new difficulty values)

## Player Input System (created Sprint 2 Phase 1)
- File: Public/Core/FrogPlayerController.h + Private/Core/FrogPlayerController.cpp
- APlayerController subclass with Enhanced Input (pure C++, no .uasset files)
- 6 input actions: IA_HopUp, IA_HopDown, IA_HopLeft, IA_HopRight, IA_Start, IA_Pause
- All actions are Boolean type, created via NewObject<UInputAction> in constructor
- IMC_Frogger mapping context created in constructor with all key bindings
- Key bindings: WASD + Arrow keys for movement, Enter/Space for Start, Escape for Pause
- Hop directions: Up=(0,1,0), Down=(0,-1,0), Left=(-1,0,0), Right=(1,0,0)
- State gating: CanAcceptHopInput() returns true only in Playing state
- Start input: works in Title (calls StartGame) and GameOver (calls ReturnToTitle then StartGame)
- Pause: stubbed for Sprint 3
- SetupInputComponent binds all actions via ETriggerEvent::Started
- BeginPlay adds IMC_Frogger to EnhancedInputLocalPlayerSubsystem
- Tests: 3 automation tests in Tests/InputSystemTest.cpp
  - ActionsCreated (all 6 non-null), MappingContextCreated, ActionValueTypes (all Boolean)
  - NewObject<AFrogPlayerController>() pattern (constructor creates actions)

## Camera System (created Sprint 2 Phase 1)
- File: Public/Core/FroggerCameraActor.h + Private/Core/FroggerCameraActor.cpp
- AActor subclass with UCameraComponent (not ACameraActor)
- Fixed position: (650, 750, 1800) -- centered above 13x15 grid
- Pitch: -72 degrees (slight angle for depth perception, not pure top-down)
- FOV: 60 degrees
- Tick disabled (PrimaryActorTick.bCanEverTick = false)
- CameraComponent is RootComponent
- Tunables: CameraPosition (FVector), CameraPitch (float), CameraFOV (float)
- BeginPlay: re-applies tunables, auto-sets as view target via PC->SetViewTarget(this)
- Tests: 4 automation tests in Tests/CameraSystemTest.cpp
  - ComponentExists, Pitch, FOV, Position
  - NewObject<AFroggerCameraActor>() pattern (no UWorld needed for constructor tests)

## Placeholder Meshes (Sprint 2 Phase 1 Task 3)
- AFrogCharacter: /Engine/BasicShapes/Sphere, scale 0.8 (~40 UU radius), green (0.1, 0.9, 0.1)
  - Mesh assigned in constructor via ConstructorHelpers::FObjectFinder
  - Dynamic material created in BeginPlay with BaseColor parameter
- AHazardBase: mesh type chosen by HazardType in SetupMeshForHazardType() called from BeginPlay
  - Road hazards (Car, Truck, Bus, Motorcycle): /Engine/BasicShapes/Cube
  - River objects (SmallLog, LargeLog, TurtleGroup): /Engine/BasicShapes/Cylinder, rotated (0, 90, 0)
  - Cube/Cylinder mesh assets cached in constructor via ConstructorHelpers, stored as UPROPERTY TObjectPtr<UStaticMesh>
  - Scale: X = HazardWidthCells, Y = 1.0, Z = 0.5 (half-cell height)
  - Colors: Car=red, Truck=dark red, Bus=orange, Motorcycle=yellow, Logs=brown, Turtles=dark green
  - Dynamic material created per-hazard with BaseColor parameter
- Tests: 2 automation tests in Tests/MeshTest.cpp
  - FrogCharacterHasMesh, HazardBaseHasMesh

## Sprint 6 Changes

### TickVFX Wiring (Commit 1)
- **Bug**: UFroggerVFXManager::TickVFX() was defined but never called from GameMode::Tick. VFX spawned static, relied on SetLifeSpan(5.0f) auto-destroy only.
- **Fix**: GameMode::Tick now calls VFX->TickVFX(GetWorld()->GetTimeSeconds()) after TickTimer. Gets VFXManager via GetGameInstance()->GetSubsystem<UFroggerVFXManager>().
- **Seam test**: FSeam_VFXTickDrivesAnimation (Seam 13 in SeamTest.cpp) -- creates VFXManager, adds FActiveVFX with null actor, calls TickVFX, verifies cleanup.
- **Pattern**: TickVFX removes entries with nullptr Actor immediately (line 171-175 of FroggerVFXManager.cpp), before checking duration. Test exploits this for verification without a UWorld.

### High Score Persistence (Commit 3, updated Sprint 7)
- **Feature**: File I/O save/load of a single integer to `Saved/highscore.txt`
- **API**: `LoadHighScore()` reads file via FFileHelper::LoadFileToString, sets HighScore if loaded value > current. `SaveHighScore()` writes HighScore via FFileHelper::SaveStringToFile.
- **Integration**: `StartNewGame()` calls `LoadHighScore()` as first line. `SaveHighScore()` is called ONLY from GameMode::HandleGameOver() and GameMode::ReturnToTitle() — NOT from NotifyScoreChanged() (Sprint 7 fix: was causing per-tick disk writes).
- **Tests**: FScore_HighScorePersistsSaveLoad (save/load round-trip), FScore_HighScoreLoadMissingFile (graceful missing file), FScore_NoPerTickSaveHighScore (verifies no disk writes during gameplay). All tests clean up via IFileManager::Get().Delete().
- **Includes added**: Misc/FileHelper.h, HAL/PlatformFilemanager.h in ScoreSubsystem.cpp

## Sprint 7 Changes

### SaveHighScore Fix (Task 1)
- **Bug**: `NotifyScoreChanged()` called `SaveHighScore()` on every score update, causing disk I/O every tick.
- **Fix**: Removed `SaveHighScore()` from `NotifyScoreChanged()`. Added calls in `HandleGameOver()` and `ReturnToTitle()`.
- **Side effect**: Tests calling `StartNewGame()` (which calls `LoadHighScore()`) can load stale highscore.txt from prior sessions. Added `IFileManager::Get().Delete()` cleanup to HighScore and NewGame tests.

### Duplicate Wave-Complete Fix (Task 2)
- **Bug**: Both `TryFillHomeSlot()` and `HandleHopCompleted()` independently checked for wave completion. Filling the last slot triggered double state transitions, double score bonuses, and two concurrent RoundComplete timers.
- **Fix**: Removed the wave-complete check from `HandleHopCompleted()`. `TryFillHomeSlot()` is the sole authority — it calls `OnWaveComplete()`. `HandleHopCompleted()` checks `CurrentState != EGameState::RoundComplete` to decide whether to start Spawning.
- **Seam test**: FSeam_LastHomeSlotNoDoubleBonuses (Seam 15) — fills 4 slots, then HandleHopCompleted for the 5th, verifies exactly 5 filled and no double transition.

### Cached Subsystem Pointers (Task 3)
- **Refactor**: Added `CachedVFXManager` and `CachedAudioManager` (TObjectPtr<>) to GameMode header. Cached in BeginPlay(). Used in Tick() and TickTimer() instead of GetGameInstance()->GetSubsystem<>() per frame.
- Forward declarations used in header; full includes remain in .cpp only.

### Sprint 7 Lessons
- **Stale disk state breaks tests**: When removing disk I/O from hot paths, tests that relied on the old write-then-read pattern may load stale files. Always clean up test artifacts at the START of tests, not just the end.
- **Concurrent UE processes conflict**: Two UnrealEditor-Cmd instances on the same project cause signal 15 (SIGTERM) from shared memory conflicts. Teammates running tests simultaneously will kill each other's processes. Need to coordinate or serialize test runs.
- **Test count**: Sprint 7 Phase 0 ended with 43 passing tests across GameState/Score/Seam (157+ total expected across all categories).
