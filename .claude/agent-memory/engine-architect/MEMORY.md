# Engine Architect Memory

*Persistent knowledge accumulated across sessions. First 200 lines loaded automatically.*

## Project Structure
- Module: UnrealFrog (primary game module)
- Public headers: Source/UnrealFrog/Public/
- Private impl: Source/UnrealFrog/Private/
- Tests: Source/UnrealFrog/Tests/
- Build.cs deps: Core, CoreUObject, Engine, InputCore, EnhancedInput

## AFrogCharacter (created session 1)
- File: Public/Core/FrogCharacter.h + Private/Core/FrogCharacter.cpp
- APawn subclass with grid-based movement
- Grid: 13 columns x 15 rows, 100 UU cell size
- Hop: 0.15s forward, 0.225s backward (1.5x multiplier), 30 UU arc height
- Input buffering during active hops
- GridPosition updated immediately on hop start (logical position = target)
- Parabolic arc: 4*alpha*(1-alpha) for Z interpolation
- Cardinal direction snapping via largest absolute axis
- Tests: 6 automation tests in Tests/FrogCharacterTest.cpp

## UScoreSubsystem (created session 2)
- File: Public/Core/ScoreSubsystem.h + Private/Core/ScoreSubsystem.cpp
- UGameInstanceSubsystem -- pure scoring logic, no gameplay dependencies
- Forward hop: PointsPerHop(10) * Multiplier, multiplier += 0.5 each consecutive hop
- Multiplier resets on: ResetMultiplier(), LoseLife()
- Time bonus: (RemainingTime / MaxTime) * 1000
- Extra life every 10,000 pts, capped at MaxLives(5), tracked via LastExtraLifeThreshold
- High score persists across StartNewGame(), score/lives/multiplier reset
- Delegates: FOnScoreChanged, FOnLivesChanged, FOnExtraLife, FOnGameOver
- Tests: 10 automation tests in Tests/ScoreSubsystemTest.cpp (written by QA lead per workflow)
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
  - Odd rows move right, even rows move left
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

## Game State Machine (created session 5)
- EGameState enum in UnrealFrogGameMode.h (not LaneTypes.h): Menu, Playing, Paused, GameOver
- File: Public/Core/UnrealFrogGameMode.h + Private/Core/UnrealFrogGameMode.cpp
- AUnrealFrogGameMode : AGameModeBase
- State machine: Menu->Playing (StartGame), Playing->Paused (PauseGame), Paused->Playing (ResumeGame), Playing/Paused->GameOver (TriggerGameOver), GameOver->Menu (RestartGame)
- Invalid transitions silently rejected (guard clauses)
- Level timer: LevelTimerMax(30s), TickTimer(dt) returns true on timeout, paused state blocks ticking
- Home slots: 5 total, FillHomeSlot() returns true on level complete, resets timer each fill
- Wave progression: AdvanceWave() increments WaveNumber, WaveSpeedMultiplier = 1.0 + 0.15*(wave-1)
- AdvanceWave resets HomeSlotsFilledCount and timer
- RestartGame resets everything except high score (that's in ScoreSubsystem)
- Delegates: FOnGameStateChanged(New, Old), FOnTimerChanged(float), FOnWaveChanged(int32)
- Tests: 9 automation tests in Tests/GameModeTest.cpp
