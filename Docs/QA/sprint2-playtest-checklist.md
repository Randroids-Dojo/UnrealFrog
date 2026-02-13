# Sprint 2 QA Play-Test Checklist

*QA Lead | Sprint 2 Final Gate*

## Prerequisites

Before running this checklist:

1. **Create the map** (one-time setup):
   ```bash
   "/Users/Shared/Epic Games/UE_5.7/Engine/Binaries/Mac/UnrealEditor-Cmd" \
     "/Users/randroid/Documents/Dev/Unreal/UnrealFrog/UnrealFrog.uproject" \
     -ExecutePythonScript="/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Tools/CreateMap/create_frog_map.py" \
     -unattended -nopause
   ```

2. **Run automated tests**:
   ```bash
   ./Tools/PlayUnreal/run-tests.sh
   ```

3. **Launch the game**:
   ```bash
   ./Tools/PlayUnreal/play-game.sh
   ```
   Or open the project in Unreal Editor and press Play.

## Acceptance Checklist

| # | Check | Method | Pass/Fail | Notes |
|---|-------|--------|-----------|-------|
| AC-1 | Game launches without errors when pressing Play | Editor PIE | | |
| AC-2 | Frog is visible as a green sphere at center-bottom of grid | Visual | | |
| AC-3 | Arrow keys move the frog one cell per press with visible arc | Input + Visual | | |
| AC-4 | Frog cannot move outside the 13x15 grid | Input at boundaries | | |
| AC-5 | Road hazards (colored boxes) move across road lanes, wrapping at edges | Visual | | |
| AC-6 | Collision with a road hazard kills the frog (frog disappears, life lost) | Deliberate collision | | |
| AC-7 | Frog respawns at start position after death (1s delay) | Wait and observe | | |
| AC-8 | River lanes show moving platforms (brown cylinders) | Visual | | |
| AC-9 | Landing on a log carries the frog with the log | Hop onto log, observe | | |
| AC-10 | Missing all logs on a river row kills the frog (splash death) | Hop to empty river | | |
| AC-11 | Being carried off-screen by a log kills the frog | Ride log to edge | | |
| AC-12 | Score increases on forward hops (visible on HUD) | Hop forward, check HUD | | |
| AC-13 | Timer bar drains during gameplay | Visual | | |
| AC-14 | Timer reaching 0 kills the frog | Wait for timeout | | |
| AC-15 | Reaching a home slot at row 14 fills it and awards points | Navigate to home slot | | |
| AC-16 | Filling all 5 home slots triggers round complete + wave 2 | Fill all 5 | | |
| AC-17 | Losing all lives shows "GAME OVER" on screen | Die until 0 lives | | |
| AC-18 | Wave 2+ hazards move faster than Wave 1 | Complete a round, observe | | |

## Defects

Any failures are logged here with reproduction steps:

| # | AC Item | Description | Severity | Status |
|---|---------|-------------|----------|--------|
| | | | | |

## Sign-off

- [ ] All 18 acceptance items passed
- [ ] No P0/P1 defects remaining
- [ ] Sprint 2 is playable end-to-end

**QA Lead signature:** _______________
**Date:** _______________
