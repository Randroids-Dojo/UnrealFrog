# Seam Test Coverage Matrix

*Last updated: Sprint 11*

## Purpose

Every pair of systems with runtime interactions must have at least one seam test
or an explicit "deferred" acknowledgment. This matrix tracks coverage.

## Legend

- **COVERED** -- at least one seam test exercises this boundary
- **DEFERRED** -- low risk, explicitly skipped with rationale
- **MISSING** -- needs a test (blocker for sprint completion)

## Matrix

| # | System A | System B | Seam Description | Status | Test Name |
|---|----------|----------|------------------|--------|-----------|
| 1 | FrogCharacter | HazardBase (Log) | Hop origin uses actual world pos on moving platform | COVERED | `FSeam_HopFromMovingPlatform` |
| 2 | FrogCharacter | HazardBase (Turtle) | Submerged turtle triggers Splash death | COVERED | `FSeam_DieOnSubmergedTurtle` |
| 3 | FrogCharacter + GameMode | HazardBase (Log) | Pause freezes timer while riding log | COVERED | `FSeam_PauseDuringRiverRide` |
| 4 | FrogCharacter | HazardBase (Log) | Hop from carried position (platform drift) | COVERED | `FSeam_HopWhilePlatformCarrying` |
| 5 | FrogCharacter | HazardBase | Death clears CurrentPlatform; respawn is clean | COVERED | `FSeam_DeathResetsCurrentPlatform` |
| 6 | GameMode | FrogPlayerController | Timer value identical before pause and after unpause | COVERED | `FSeam_TimerFreezesDuringPause` |
| 7 | FrogCharacter | HazardBase (Turtle) | Turtle submerging while frog rides triggers death | COVERED | `FSeam_TurtleSubmergeWhileRiding` |
| 8 | AudioManager | GameMode | Music switches track on game state change | COVERED | `FSeam_MusicSwitchesOnStateChange` |
| 9 | VFXManager | FrogCharacter | VFX spawns on hop start | COVERED | `FSeam_VFXSpawnsOnHopStart` |
| 10 | VFXManager | FrogCharacter | VFX spawns on death | COVERED | `FSeam_VFXSpawnsOnDeath` |
| 11 | HUD | ScoreSubsystem | Score delta detection triggers pop animation | COVERED | `FSeam_HUDScorePopOnScoreIncrease` |
| 12 | AudioManager | bMuted flag | Music respects mute flag | COVERED | `FSeam_MusicMutedInCI` |
| 13 | GameMode | ScoreSubsystem | StartGame resets score/lives via StartNewGame | DEFERRED | Low risk -- direct call, tested in isolation |
| 14 | GameMode | LaneManager | Wave difficulty applies speed multiplier + gap reduction to lane configs | COVERED | `FSeam_WaveDifficultyFlowsToLaneConfig` |
| 15 | FrogCharacter | FrogPlayerController | Input buffering during hop | DEFERRED | Low risk -- single system boundary, tested in FrogCharacter isolation tests |
| 15b | GameMode | HomeSlots | Landing on already-filled home slot kills frog (GOAL-04) | COVERED | `FSeam_FilledHomeSlotCausesDeath` |
| 15c | GameMode | HomeSlots | Landing on non-home-slot column at goal row kills frog (GOAL-03) | COVERED | `FSeam_NonHomeSlotColumnCausesDeath` |
| 16 | ScoreSubsystem | AudioManager | Extra life sound fires on threshold cross | DEFERRED | Low risk -- single AddDynamic binding, tested in wiring tests |
| 17 | GroundBuilder | LaneManager | Ground rows match lane config rows | DEFERRED | Low risk -- both read from same constants |
| 18 | VFXManager | GameMode (Tick) | TickVFX driven from GameMode::Tick for animation | COVERED | `FSeam_VFXTickDrivesAnimation` |
| 19 | AudioManager | GameMode (Timer) | Timer warning sound fires at <16.7% threshold | COVERED | `FSeam_TimerWarningFiresAtThreshold` |
| 20 | PlayUnreal (Python) | GameMode (GetGameStateJSON) | Remote Control calls GetGameStateJSON; returned JSON has score, lives, wave, frogPos, gameState, timeRemaining, homeSlotsFilledCount | COVERED | `FPlayUnreal_GetGameStateJSON` + `Tools/PlayUnreal/acceptance_test.py` |
| 21 | PlayUnreal (Python) | FrogCharacter (RequestHop) | Remote Control invokes RequestHop; frog position advances by one grid cell | COVERED | `FPlayUnreal_ForwardProgress` + `Tools/PlayUnreal/acceptance_test.py` |
| 22 | FroggerHUD | APlayerController | Score pop position uses ProjectWorldLocationToScreen on frog world location, not hardcoded screen coords | COVERED | `FHUD_ScorePopUsesWorldProjection` |
| 23 | FroggerVFXManager | Camera (distance + FOV) | CalculateScaleForScreenSize computes scale from camera distance and FOV; death puff StartScale >= 5% of visible width | COVERED | `FVFX_DeathPuffScaleForCameraDistance` |
| 24 | FroggerVFXManager | GameMode (HomeSlotColumns, HomeSlotRow) | Home slot celebration positions derived from GameMode grid config, not hardcoded magic numbers | COVERED | `FVFX_HomeSlotSparkleReadsGridConfig` |
| 25 | Difficulty scaling | Human perception threshold | Cumulative audio pitch + ground color change >= perceptible threshold by Wave 3 (5-second test) | DEFERRED | Game Designer implementing perception signals (Tasks 13-15) -- update to COVERED when tests land |
| 26 | FrogCharacter | PlatformLandingMargin | FindPlatformAtCurrentPosition uses PlatformLandingMargin UPROPERTY (not capsule radius) for effective detection width; 10 boundary tests at center, 25%, margin edge, outside, raw edge, wrong row, non-rideable, two platforms, negative side, submerged | COVERED | `FCollision_FindPlatform_*` (10 tests in CollisionTest.cpp) |
| 27 | GameMode (GetGameConfigJSON) | Python constants (path_planner.py) | GetGameConfigJSON returns live UPROPERTY values; Python init_from_config() overwrites module constants; --check-sync verifies SYNC annotations match C++ defaults | COVERED | `FGameConfig_*` (3 tests) + `run-tests.sh --check-sync` |
