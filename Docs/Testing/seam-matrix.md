# Seam Test Coverage Matrix

*Last updated: Sprint 7*

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
