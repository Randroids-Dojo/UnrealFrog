# Conversation: 2026-02-16

| Field | Value |
|-------|-------|
| Date | 2026-02-16 |
| Session ID | `486a1fce-113f-4722-9497-667c8c9ccddd` |
| Model | claude-sonnet-4-5-20250929 |
| Tokens | ~489,132 input, ~12 output |
| Duration | 1m 6s |

---

## User (03:59 UTC)

Research the UnrealFrog lane layout, hazard speeds, and timing to help script intelligent frog movement that avoids hazards.

Find and read these files:
1. `Docs/Design/sprint1-gameplay-spec.md` — grid system, lane layout, movement rules
2. `Source/UnrealFrog/Public/Core/LaneManager.h` — lane configurations, hazard speeds
3. `Source/UnrealFrog/Private/Core/LaneManager.cpp` — how lanes are set up, hazard spacing/timing
4. `Source/UnrealFrog/Public/Core/LaneTypes.h` — lane type definitions, FLaneConfig
5. `Source/UnrealFrog/Public/Core/FrogCharacter.h` — HopDuration, grid size
6. `Source/UnrealFrog/Private/Core/UnrealFrogGameMode.cpp` — lane setup in BeginPlay, which rows are road/river/safe
7. `Source/UnrealFrog/Public/Core/HomeSlotManager.h` — home slot positions

Extract:
- Grid dimensions (columns x rows)
- Which rows are safe zones, road lanes, river lanes, home row
- Hazard speeds per lane (or base speed + difficulty multiplier)
- Hazard spacing/gaps between vehicles/logs
- Frog hop duration
- Grid cell size (for calculating timing windows)
- River log positions and widths (for safe landing)
- Home slot X positions

The goal is to calculate: "after hopping into row Y, how long do I need to wait before it's safe to hop to row Y+1?" and "which X column should I be in to land on a river log?"

Return all findings in a structured summary.

## Assistant (03:59 UTC)

🤖️

I'll search for and analyze the UnrealFrog lane layout, hazard speeds, and timing information to help script intelligent frog movement.

- **Used Read** on `Docs/Design/sprint1-gameplay-spec.md`

- **Used Read** on `Source/UnrealFrog/Public/Core/LaneManager.h`

- **Used Read** on `Source/UnrealFrog/Private/Core/LaneManager.cpp`

- **Used Read** on `Source/UnrealFrog/Public/Core/LaneTypes.h`

- **Used Read** on `Source/UnrealFrog/Public/Core/FrogCharacter.h`

- **Used Read** on `Source/UnrealFrog/Private/Core/UnrealFrogGameMode.cpp`

- **Used Read** on `Source/UnrealFrog/Public/Core/HomeSlotManager.h`

## Assistant (04:00 UTC)

Let me search for the HomeSlotManager files.

- **Used Glob** for `**/HomeSlot*.h` in `.`

- **Used Glob** for `**/HomeSlot*.cpp` in `.`

## Assistant (04:00 UTC)

I can see from the GameMode code that home slots are managed directly in the GameMode, not in a separate manager. Let me extract the complete information now.

## UnrealFrog Lane Layout and Movement Intelligence Report

### Grid Configuration
- **Grid dimensions**: 13 columns (0-12) × 15 rows (0-14)
- **Cell size**: 100 Unreal Units (UU)
- **World-space mapping**: Grid origin (0,0) → World (-600, 0, 0)
  - Formula: `World.X = (Column - 6) * 100`, `World.Y = Row * 100`
- **Center column**: 6 (frog spawn location)

### Zone Layout (Bottom to Top)

| Row | Zone Type | Description |
|-----|-----------|-------------|
| 0 | Safe (Start) | Frog spawn zone, no hazards |
| 1-5 | Road | Traffic lanes with deadly vehicles |
| 6 | Safe (Median) | Rest zone, no hazards |
| 7-12 | River | Deadly water with moving platforms |
| 13 | Goal (lower) | Frog passes through |
| 14 | Goal (upper) | Home slots at columns 1, 4, 6, 8, 11 |

### Frog Movement Parameters
- **HopDuration**: 0.15 seconds (current tuned value: 0.08s for InputBufferWindow)
- **HopArcHeight**: 30 UU
- **InputBufferWindow**: 0.08 seconds before hop completes
- **GridCellSize**: 100 UU

### Road Hazards (Rows 1-5) — Base Speeds (Wave 1)

| Row | Type | Direction | Speed (UU/s) | Width (cells) | Gap (cells) | Count |
|-----|------|-----------|-------------|---------------|-------------|-------|
| 1 | Car | LEFT (-X) | 150 | 1 | 3 | 3 |
| 2 | Truck | RIGHT (+X) | 100 | 2 | 4 | 3 |
| 3 | Car | LEFT (-X) | 200 | 1 | 2 | 3 |
| 4 | Bus | RIGHT (+X) | 175 | 2 | 3 | 3 |
| 5 | Motorcycle | LEFT (-X) | 250 | 1 | 2 | 3 |

**Critical timing notes for road hazards:**
- Collision bounds are **smaller than visual** (80% of visual size) — near-misses are intentional
- Hazards wrap around seamlessly when exiting the playfield
- All road hazards are DEADLY on contact

### River Platforms (Rows 7-12) — Base Speeds (Wave 1)

| Row | Type | Direction | Speed (UU/s) | Width (cells) | Gap (cells) | Count |
|-----|------|-----------|-------------|---------------|-------------|-------|
| 7 | SmallLog | RIGHT (+X) | 100 | 2 | 2 | 3 |
| 8 | TurtleGroup | LEFT (-X) | 80 | 3 | 2 | 3 |
| 9 | LargeLog | RIGHT (+X) | 120 | 4 | 2 | 3 |
| 10 | SmallLog | LEFT (-X) | 100 | 2 | 2 | 3 |
| 11 | TurtleGroup | RIGHT (+X) | 80 | 3 | 2 | 3 |
| 12 | LargeLog | LEFT (-X) | 150 | 4 | 2 | 3 |

**Critical timing notes for river platforms:**
- Platform bounds are **larger than visual** (generous landing) — opposite of road hazards
- Landing in water (missing all platforms) = instant death
- Frog moves WITH the platform while riding (world position drifts between hops)
- Carried off-screen (past column 0 or 12) = death
- Platform widths: SmallLog = 2 cells (200 UU), LargeLog = 4 cells (400 UU), Turtle = 3 cells (300 UU)

### Home Slots (Row 14)
- **Valid columns**: 1, 4, 6, 8, 11
- Landing on row 14 at any OTHER column = death (bushes/water)
- Each slot can only be filled once per round
- Filling all 5 slots triggers wave completion

### Difficulty Scaling (Wave Progression)

**Speed multiplier formula** (from GameMode):
```
SpeedMultiplier = min(2.0, 1.0 + (Wave - 1) * 0.15)
Actual Speed = BaseSpeed * SpeedMultiplier
```

| Wave | Speed Multiplier | Gap Reduction |
|------|-----------------|---------------|
| 1 | 1.00× | 0 cells |
| 2 | 1.15× | 0 cells |
| 3 | 1.30× | 1 cell |
| 4 | 1.45× | 1 cell |
| 5 | 1.60× | 2 cells |
| 6 | 1.75× | 2 cells |
| 7 | 1.90× | 3 cells |
| 8+ | 2.00× (cap) | 3+ cells |

**Gap reduction**: `(Wave - 1) / 2` cells (integer division)

### Hazard Spawn Distribution

From `LaneManager::CalculateSpawnPositions()`:
- Hazards are evenly spaced across the lane width
- Spacing formula: `GridWorldWidth / NumHazards`
- NumHazards is clamped to `min(HazardsPerLane, floor(GridWidth / (HazardWidth + Gap)))`
- Default `HazardsPerLane` = 3

For a 13-cell-wide grid (1300 UU):
- **Row 1 (Car, gap 3)**: 3 cars, spacing ~433 UU
- **Row 3 (Car, gap 2)**: 3 cars, spacing ~433 UU  
- **Row 5 (Motorcycle, gap 2)**: 3 motorcycles, spacing ~433 UU

### Timing Windows for Safe Movement

**Key calculation**: How long does it take for a gap to pass?

For a hazard moving at speed `S` UU/s with gap width `G` cells (100×G UU):
- **Gap crossing time** = `(G × 100) / S` seconds
- **Hazard crossing time** = `(W × 100) / S` seconds (where W = hazard width in cells)

**Example (Row 5: Motorcycle at 250 UU/s, gap 2 cells, Wave 1):**
- Gap width: 200 UU
- Gap crossing time: 200 / 250 = **0.8 seconds**
- Motorcycle width: 100 UU  
- Motorcycle crossing time: 100 / 250 = **0.4 seconds**
- Frog hop duration: **0.15 seconds**

**Safe hop window**: You need the gap to be aligned with your column for at least the hop duration (0.15s). Since gaps cross in 0.8s and motorcycles in 0.4s, you have multiple safe windows per cycle.

**Example (Row 12: LargeLog at 150 UU/s → 300 UU/s at Wave 8, width 4 cells):**
- At Wave 8: Speed = 150 × 2.0 = **300 UU/s**
- Log width: 400 UU
- Log crossing time: 400 / 300 = **1.33 seconds** (safe landing window on the log)
- Gap width: 200 UU (gap reduced by 1 cell at Wave 5+)
- Gap crossing time: 200 / 300 = **0.67 seconds** (danger window)

### Critical Path Calculations

**Absolute file paths referenced:**
- `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Docs/Design/sprint1-gameplay-spec.md` — Complete gameplay spec
- `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Public/Core/LaneManager.h` — Lane configuration interface
- `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Private/Core/LaneManager.cpp` — Default lane setup and spawn logic
- `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Public/Core/LaneTypes.h` — Lane type enums and FLaneConfig struct
- `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Public/Core/FrogCharacter.h` — Frog movement parameters
- `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Private/Core/UnrealFrogGameMode.cpp` — Game orchestration, home slots at columns {1,4,6,8,11}

### Intelligent Movement Strategy Recommendations

**For road hazards:**
1. Calculate time until gap aligns with your column: `(GapX - FrogX) / Speed`
2. If time > HopDuration (0.15s), hop is safe
3. Account for hazard width — ensure entire frog landing zone is clear

**For river platforms:**
1. Track platform center X positions over time
2. Calculate when platform will align with your column
3. Ensure landing occurs while platform is overlapping your target cell
4. Account for drift — frog's actual X position changes while riding, so `GridPosition` becomes stale
5. Next hop origin must be `GetActorLocation()` not `GridToWorld(GridPosition)`

**For home slots (row 14):**
- Only hop to row 14 if your column is one of {1, 4, 6, 8, 11}
- Otherwise, hop sideways first to align with a valid slot column

**Wave-aware timing:**
- Query GameMode's `GetSpeedMultiplier()` to get current wave scaling
- Multiply all base speeds by this factor when calculating timing windows
- At Wave 8 (max difficulty), all hazards move 2× faster — gaps close in half the time
