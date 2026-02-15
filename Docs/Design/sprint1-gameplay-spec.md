# Sprint 1 Gameplay Specification

*Author: Game Designer*
*Sprint: 1 -- Core Grid, Movement, and Hazard Foundation*
*Status: ACTIVE*

---

## 1. Grid System

### Cell Size

- **GridCellSize**: 100 Unreal Units (UU)
- Rationale: 100 UU is one "meter" in UE5 default scale. Clean math, easy debugging. A frog occupies exactly 1 cell.

### Grid Dimensions

- **Columns**: 13 (indices 0-12)
- **Rows**: 15 (indices 0-14, bottom to top)
- **Total playfield**: 1300 UU wide x 1500 UU deep

### World-Space Mapping

Grid origin `(0, 0)` maps to world position `(-600, 0, 0)`.

The grid is oriented along the X (left-right) and Y (forward-back) axes. Z is up.

```
World.X = (GridColumn - 6) * 100.0
World.Y = GridRow * 100.0
World.Z = 0.0 (ground plane)
```

| Grid Coord | World Position | Notes |
|-----------|----------------|-------|
| (0, 0) | (-600, 0, 0) | Bottom-left corner |
| (6, 0) | (0, 0, 0) | Bottom-center (spawn) |
| (12, 0) | (600, 0, 0) | Bottom-right corner |
| (6, 14) | (0, 1400, 0) | Top-center (goal zone) |
| (0, 14) | (-600, 1400, 0) | Top-left corner |
| (12, 14) | (600, 1400, 0) | Top-right corner |

The center column is 6. The frog spawns at grid position `(6, 0)`.

### Coordinate Helper (C++ Reference)

```cpp
// Grid to World
FVector GridToWorld(int32 Col, int32 Row)
{
    return FVector((Col - 6) * GridCellSize, Row * GridCellSize, 0.0f);
}

// World to Grid
FIntPoint WorldToGrid(FVector WorldPos)
{
    return FIntPoint(
        FMath::RoundToInt(WorldPos.X / GridCellSize) + 6,
        FMath::RoundToInt(WorldPos.Y / GridCellSize)
    );
}
```

---

## 2. Zone Layout (Bottom to Top)

Each row has a designated zone type. Zones determine what hazards (or safety) a row provides.

| Row | Zone | Description |
|-----|------|-------------|
| 0 | **Start** | Safe spawn zone. Frog starts here. No hazards ever. |
| 1 | **Road** | Traffic Lane 1 -- vehicles move LEFT |
| 2 | **Road** | Traffic Lane 2 -- vehicles move RIGHT |
| 3 | **Road** | Traffic Lane 3 -- vehicles move LEFT |
| 4 | **Road** | Traffic Lane 4 -- vehicles move RIGHT |
| 5 | **Road** | Traffic Lane 5 -- vehicles move LEFT |
| 6 | **Median** | Safe rest zone. No hazards. Visual: sidewalk/grass strip. |
| 7 | **River** | River Lane 1 -- platforms move RIGHT |
| 8 | **River** | River Lane 2 -- platforms move LEFT |
| 9 | **River** | River Lane 3 -- platforms move RIGHT |
| 10 | **River** | River Lane 4 -- platforms move LEFT |
| 11 | **River** | River Lane 5 -- platforms move RIGHT |
| 12 | **River** | River Lane 6 -- platforms move LEFT |
| 13 | **Goal** | Goal Row (lower) -- frog passes through |
| 14 | **Goal** | Goal Row (upper) -- 5 home slots at specific columns |

### Goal Zone Detail

Row 14 contains 5 home slots (lily pads) at specific column positions. The frog must land exactly on a home slot to score. Landing on row 14 outside a home slot counts as falling in the river (death).

**Home Slot Columns**: 1, 4, 6, 8, 11

These are evenly-ish distributed across the 13 columns with gaps between them. Each slot is 1 cell wide.

```
Row 14: [_][H][_][_][H][_][H][_][H][_][_][H][_]
Col:     0  1  2  3  4  5  6  7  8  9  10 11 12
```

A home slot can only be filled once per round. Once all 5 are filled, the round is complete.

---

## 3. Movement Parameters

All values exposed as `UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")` on `AFrogCharacter`.

| Parameter | Type | Value | Description |
|-----------|------|-------|-------------|
| `GridCellSize` | float | 100.0 | Size of one grid cell in UU |
| `HopDuration` | float | 0.15 | Seconds to complete one hop animation |
| `HopArcHeight` | float | 30.0 | Peak Z offset during hop (parabolic arc) |
| `InputBufferWindow` | float | 0.1 | Seconds before hop completes that the next input is queued |
| `MovementCooldown` | float | 0.0 | Additional delay after hop completes before next hop (set to 0 for responsive arcade feel) |
| `RespawnDelay` | float | 1.0 | Seconds after death before frog reappears at start |

### Movement Rules

1. **Grid-locked**: The frog always moves exactly 1 cell (100 UU) per hop in one of 4 cardinal directions.
2. **No diagonal movement**: Only one axis changes per hop.
3. **Boundary clamping**: The frog cannot hop outside the grid. Column range: 0-12. Row range: 0-14. Inputs that would move the frog out of bounds are ignored (no wrap-around).
4. **Forward and backward hop speed are identical**: `HopDuration` = 0.15s in all directions. (The scoring system incentivizes forward movement instead.)
5. **Hop animation**: Smooth interpolation from start cell to end cell with a parabolic Z arc peaking at `HopArcHeight`.
6. **Input during hop**: If the player presses a direction during the last `InputBufferWindow` (0.1s) of a hop, that input is queued and executed immediately when the current hop completes.
7. **One buffered input maximum**: Only the most recent buffered input is stored. Earlier buffered inputs are overwritten.

### Hop Arc Formula

```
Progress = ElapsedTime / HopDuration    // 0.0 to 1.0
Z_Offset = HopArcHeight * 4.0 * Progress * (1.0 - Progress)    // Parabola: peaks at 0.5
XY_Position = FMath::Lerp(StartPos, EndPos, Progress)
```

This produces a smooth arc that starts at Z=0, peaks at `HopArcHeight` (30 UU) at the halfway point, and returns to Z=0.

---

## 4. Hazard Parameters

### 4.1 Road Hazards (Rows 1-5)

Road hazards kill the frog on contact. They move continuously in their lane direction and wrap around when they exit the playfield.

**Wave 1 (Starting Configuration)**:

| Row | Hazard Type | Direction | Speed (UU/s) | Width (cells) | Gap (cells) | Spawn Count |
|-----|-------------|-----------|-------------|---------------|-------------|-------------|
| 1 | Car | LEFT | 150 | 1 | 3 | 3 |
| 2 | Truck | RIGHT | 100 | 2 | 4 | 2 |
| 3 | Car | LEFT | 200 | 1 | 2 | 4 |
| 4 | Bus | RIGHT | 175 | 2 | 3 | 2 |
| 5 | Motorcycle | LEFT | 250 | 1 | 2 | 3 |

**Hazard Type Definitions**:

| Type | Base Speed (UU/s) | Width (cells) | Collision Bounds |
|------|-------------------|---------------|-----------------|
| Car | 150 | 1 | 80 x 80 x 60 UU (centered in cell) |
| Truck | 100 | 2 | 180 x 80 x 80 UU |
| Bus | 175 | 2 | 180 x 80 x 90 UU |
| Motorcycle | 250 | 1 | 60 x 60 x 50 UU |

- **Collision bounds** are intentionally slightly smaller than visual bounds. This makes near-misses feel like skillful dodges rather than unfair hits. (Miyamoto principle: deaths must feel fair.)
- **Wrapping**: When a hazard fully exits one side of the playfield (all cells past column -1 or column 13), it reappears on the opposite side. There is no gap -- the lane is a continuous loop.

### 4.2 River Objects (Rows 7-12)

River lanes are deadly by default. The frog must land ON a river object to survive. Missing all objects means falling in the water (death). While riding, the frog moves with the object.

**Wave 1 (Starting Configuration)**:

| Row | Object Type | Direction | Speed (UU/s) | Width (cells) | Gap (cells) | Spawn Count |
|-----|-------------|-----------|-------------|---------------|-------------|-------------|
| 7 | Log (small) | RIGHT | 100 | 2 | 3 | 3 |
| 8 | Turtle Group | LEFT | 80 | 3 | 3 | 3 |
| 9 | Log (large) | RIGHT | 120 | 4 | 3 | 2 |
| 10 | Log (small) | LEFT | 100 | 2 | 2 | 4 |
| 11 | Turtle Group | RIGHT | 80 | 3 | 4 | 2 |
| 12 | Log (large) | LEFT | 150 | 4 | 2 | 2 |

**River Object Definitions**:

| Type | Base Speed (UU/s) | Width (cells) | Platform Bounds |
|------|-------------------|---------------|-----------------|
| Log (small) | 100 | 2 | 180 x 80 UU (slightly generous) |
| Log (large) | 120 | 4 | 380 x 80 UU (slightly generous) |
| Turtle Group | 80 | 3 | 280 x 80 UU |

- **Platform bounds are slightly larger than visual bounds** (opposite of road hazards). This makes landing on platforms feel generous. The player should feel skillful, not cheated.
- **Turtle submerge mechanic** (Wave 2+, NOT Sprint 1): Turtle groups periodically sink below water for 2.0 seconds, becoming deadly. Visual warning: turtles bob lower for 1.0 second before submerging. Sprint 1 turtles are always solid.
- **Riding**: When the frog stands on a river object, it inherits the object's velocity. The frog's grid position updates each frame to match. If the frog is carried past column 0 or column 12, it dies (carried off-screen).
- **River landing**: When the frog hops into a river row, the system checks whether any river object's platform bounds overlap the frog's landing position. If yes, the frog mounts the object. If no, the frog dies (splash).

---

## 5. Scoring Rules

### Points Per Action

| Action | Points | Notes |
|--------|--------|-------|
| Forward hop (toward goal) | 10 | Only row-increasing hops. Sideways = 0, backward = 0 |
| Filling a home slot | 200 | Bonus when frog reaches an empty home slot |
| Completing a round (all 5 homes) | 1000 | Bonus on top of the 5 x 200 |
| Time bonus (per round) | `RemainingSeconds * 10` | Awarded when all 5 homes are filled |

### Forward Hop Multiplier

The multiplier rewards consecutive forward progress without retreating.

| Condition | Effect |
|-----------|--------|
| Forward hop | Multiplier increases by 1 (max 5x) |
| Sideways hop | Multiplier unchanged |
| Backward hop | Multiplier resets to 1x |
| Death | Multiplier resets to 1x |
| Reaching a home slot | Multiplier resets to 1x (new attempt starts) |

**Score per forward hop** = 10 * CurrentMultiplier

Example sequence: Forward(10), Forward(20), Forward(30), Sideways(0), Forward(40), Backward(0 + reset), Forward(10)...

Maximum theoretical multiplier: 5x. Forward hops beyond 5 consecutive still award 50 points each (capped at 5x).

### Extra Lives

- **Extra life at every 10,000 points**
- Maximum lives: 9 (display constraint -- classic arcade ceiling)
- Starting lives: 3
- Visual + audio feedback when an extra life is awarded

### High Score

- Persistent per session (not saved to disk in Sprint 1)
- Displayed on HUD at all times

---

## 6. Game State Transitions

### States

| State | Description |
|-------|-------------|
| `Title` | Title screen. Waiting for player input to start. |
| `Spawning` | Frog is respawning (brief invulnerable animation). Duration: `RespawnDelay` (1.0s). |
| `Playing` | Active gameplay. Timer counting down. Player has control. |
| `Dying` | Death animation playing. Duration: 0.5s. Player has no control. |
| `RoundComplete` | All 5 homes filled. Score tallied. Duration: 2.0s celebration. |
| `GameOver` | All lives lost. Display final score. Wait for input. |

### Transition Table

| From | To | Trigger |
|------|----|---------|
| `Title` | `Spawning` | Player presses any direction key or Start |
| `Spawning` | `Playing` | Spawn animation completes (1.0s) |
| `Playing` | `Dying` | Frog hits road hazard, falls in water, carried off-screen, or timer hits 0 |
| `Dying` | `Spawning` | Death animation completes (0.5s) AND lives > 0 |
| `Dying` | `GameOver` | Death animation completes (0.5s) AND lives == 0 |
| `Playing` | `RoundComplete` | 5th home slot filled |
| `RoundComplete` | `Spawning` | Celebration completes (2.0s). Wave increments. Timer resets. Home slots cleared. |
| `GameOver` | `Title` | Player presses Start (or any key after 3.0s delay) |

### State Diagram (ASCII)

```
[Title] --start--> [Spawning] --timer--> [Playing]
                      ^                     |
                      |                     |-- death --> [Dying]
                      |                     |               |
                      |                     |            lives>0 --> [Spawning]
                      |                     |            lives==0 -> [GameOver] --> [Title]
                      |                     |
                      |                     |-- 5 homes --> [RoundComplete]
                      |                                         |
                      +-----------------------------------------+
```

### Timer

- **TimePerLevel**: 30.0 seconds
- Timer begins counting down when state enters `Playing`
- Timer pauses during `Dying`, `Spawning`, and `RoundComplete`
- Timer resets to 30.0 at the start of each round (all 5 homes filled)
- Timer does NOT reset when filling individual home slots or dying
- Timer reaching 0 triggers death (same as any other death -- lose a life)

### Wave Progression

When a round completes (all 5 homes filled), the wave number increments. Difficulty scales:

```
SpeedMultiplier = 1.0 + (WaveNumber - 1) * 0.15
GapReduction = FMath::Max(1, BaseGap - FMath::FloorToInt((WaveNumber - 1) / 2))
```

- **Wave 1**: All hazards at base speed, base gaps
- **Wave 2**: Speeds at 1.15x, same gaps
- **Wave 3**: Speeds at 1.30x, gaps reduced by 1 cell
- **Wave 4**: Speeds at 1.45x, gaps reduced by 1 cell
- **Wave 5**: Speeds at 1.60x, gaps reduced by 2 cells
- **Speed cap**: 2.0x (at Wave 8)
- **Minimum gap**: 1 cell (never 0 -- the game must always be passable)

---

## 7. Acceptance Criteria

### 7.1 Grid System

- [ ] **GRID-01**: `GridToWorld(0, 0)` returns `(-600, 0, 0)`.
- [ ] **GRID-02**: `GridToWorld(6, 0)` returns `(0, 0, 0)`.
- [ ] **GRID-03**: `GridToWorld(12, 14)` returns `(600, 1400, 0)`.
- [ ] **GRID-04**: `WorldToGrid(FVector(0, 0, 0))` returns `(6, 0)`.
- [ ] **GRID-05**: `WorldToGrid(GridToWorld(Col, Row))` returns `(Col, Row)` for all valid grid positions (round-trip).
- [ ] **GRID-06**: Grid positions outside 0-12 columns or 0-14 rows are rejected by the movement system.

### 7.2 Movement

- [ ] **MOVE-01**: Frog spawns at grid position (6, 0).
- [ ] **MOVE-02**: Pressing Up moves the frog from (6, 0) to (6, 1) over exactly 0.15 seconds.
- [ ] **MOVE-03**: During a hop, the frog's Z position follows a parabolic arc peaking at 30.0 UU at the midpoint.
- [ ] **MOVE-04**: Pressing Down at row 0 produces no movement (boundary clamp).
- [ ] **MOVE-05**: Pressing Left at column 0 produces no movement (boundary clamp).
- [ ] **MOVE-06**: Pressing Right at column 12 produces no movement (boundary clamp).
- [ ] **MOVE-07**: Input during the last 0.1s of a hop is buffered and executed immediately after the current hop completes.
- [ ] **MOVE-08**: Only the most recent buffered input is retained (earlier buffered inputs overwritten).
- [ ] **MOVE-09**: No input is accepted during the first 0.05s of a hop (prevents accidental double-tap).
- [ ] **MOVE-10**: Frog cannot move during `Dying`, `Spawning`, `RoundComplete`, or `GameOver` states.

### 7.3 Road Hazards

- [ ] **ROAD-01**: Frog dies when its collision bounds overlap a road hazard's collision bounds.
- [ ] **ROAD-02**: Road hazards wrap from one side of the playfield to the other seamlessly.
- [ ] **ROAD-03**: Hazard collision bounds are smaller than visual bounds (80% of visual size).
- [ ] **ROAD-04**: Each road lane has exactly the hazard type, count, direction, and speed specified in Section 4.1 for Wave 1.
- [ ] **ROAD-05**: Frog standing on the Start zone (row 0) or Median (row 6) is never hit by road hazards.

### 7.4 River System

- [ ] **RIVER-01**: Frog dies when landing on a river row (7-12) without overlapping any river object's platform bounds.
- [ ] **RIVER-02**: Frog successfully mounts a river object when landing within its platform bounds.
- [ ] **RIVER-03**: While mounted, the frog moves with the river object's velocity each frame.
- [ ] **RIVER-04**: Frog dies when carried past column 0 or column 12 by a river object.
- [ ] **RIVER-05**: River object platform bounds are larger than visual bounds (slightly generous landing).
- [ ] **RIVER-06**: Each river lane has exactly the object type, count, direction, and speed specified in Section 4.2 for Wave 1.

### 7.5 Goal Zone

- [ ] **GOAL-01**: Home slots exist at columns 1, 4, 6, 8, 11 on row 14.
- [ ] **GOAL-02**: Frog landing on a home slot fills it and awards 200 points.
- [ ] **GOAL-03**: Frog landing on row 14 outside a home slot dies (water death).
- [ ] **GOAL-04**: Frog landing on an already-filled home slot dies.
- [ ] **GOAL-05**: Filling all 5 home slots triggers `RoundComplete` state and awards 1000 bonus points + time bonus.

### 7.6 Scoring

- [ ] **SCORE-01**: Forward hop awards `10 * CurrentMultiplier` points.
- [ ] **SCORE-02**: Sideways hop awards 0 points and does not change the multiplier.
- [ ] **SCORE-03**: Backward hop awards 0 points and resets the multiplier to 1.
- [ ] **SCORE-04**: Multiplier caps at 5x.
- [ ] **SCORE-05**: Death resets the multiplier to 1.
- [ ] **SCORE-06**: Reaching a home slot resets the multiplier to 1.
- [ ] **SCORE-07**: Time bonus = `RemainingSeconds * 10` (integer truncation), awarded on round completion.
- [ ] **SCORE-08**: Extra life awarded at every 10,000 point threshold.
- [ ] **SCORE-09**: Maximum lives is 9.
- [ ] **SCORE-10**: Starting lives is 3.

### 7.7 Game State

- [ ] **STATE-01**: Game begins in `Title` state.
- [ ] **STATE-02**: Any directional input during `Title` transitions to `Spawning`.
- [ ] **STATE-03**: `Spawning` lasts exactly 1.0 second, then transitions to `Playing`.
- [ ] **STATE-04**: Death in `Playing` transitions to `Dying` for 0.5 seconds.
- [ ] **STATE-05**: After `Dying`, if lives > 0, transitions to `Spawning`. If lives == 0, transitions to `GameOver`.
- [ ] **STATE-06**: `RoundComplete` lasts 2.0 seconds, increments wave, resets timer and home slots, then transitions to `Spawning`.
- [ ] **STATE-07**: Timer counts down from 30.0 during `Playing` only.
- [ ] **STATE-08**: Timer reaching 0 triggers death.
- [ ] **STATE-09**: Timer resets to 30.0 only on round completion (not on individual home slot fill or death).
- [ ] **STATE-10**: `GameOver` transitions to `Title` on input after a 3.0 second delay.

### 7.8 Wave Progression

- [ ] **WAVE-01**: Wave 1 uses base speeds and base gaps from Section 4.
- [ ] **WAVE-02**: Wave 2 multiplies all hazard speeds by 1.15.
- [ ] **WAVE-03**: Speed multiplier increases by 0.15 per wave, capping at 2.0.
- [ ] **WAVE-04**: Gaps reduce by 1 cell every 2 waves, with a minimum of 1 cell.
- [ ] **WAVE-05**: The game is always passable -- no lane configuration creates an impossible gap.

---

## Appendix A: Data-Driven Lane Configuration

All lane data should be stored in a `UDataTable` or `TArray<FLaneConfig>` so designers can tune without recompiling.

```cpp
USTRUCT(BlueprintType)
struct FLaneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
    int32 Row = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
    EZoneType ZoneType = EZoneType::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
    EHazardType HazardType = EHazardType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
    ELaneDirection Direction = ELaneDirection::Left;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
    int32 HazardWidth = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
    int32 GapSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
    int32 SpawnCount = 3;
};
```

### Enums

```cpp
UENUM(BlueprintType)
enum class EZoneType : uint8
{
    Safe,
    Road,
    River,
    Goal
};

UENUM(BlueprintType)
enum class EHazardType : uint8
{
    None,
    Car,
    Truck,
    Bus,
    Motorcycle,
    LogSmall,
    LogLarge,
    TurtleGroup
};

UENUM(BlueprintType)
enum class ELaneDirection : uint8
{
    Left,
    Right
};

UENUM(BlueprintType)
enum class EGameState : uint8
{
    Title,
    Spawning,
    Playing,
    Dying,
    RoundComplete,
    GameOver
};
```

---

## Appendix B: Sprint 1 Scope Boundary

**IN scope for Sprint 1**:
- Grid system with coordinate conversion
- Frog movement with hop animation and input buffering
- Road hazards (all 4 types) with collision
- River objects (logs and turtles, NO submerge mechanic) with riding
- Goal zone with 5 home slots
- Scoring with multiplier
- Game state machine (all 6 states)
- Timer system
- Wave progression (speed + gap scaling)
- Lives system
- Placeholder visuals (white box meshes are acceptable)

**OUT of scope for Sprint 1**:
- Turtle submerge mechanic (Wave 2+ feature)
- Lily pads (stationary river objects)
- Sound effects and music
- Particle effects
- HUD/UI beyond debug text
- Camera system (fixed top-down for Sprint 1)
- Persistent high scores
- Pause menu
- Multiple frog skins/characters

---

## Appendix C: Feel Notes for Implementers

1. **The hop must feel snappy.** 0.15s is fast. If it feels sluggish during testing, the animation curve may need to be sharpened (ease-out rather than linear). But the duration should not be changed without a design review.

2. **Near-misses are content.** The smaller-than-visual hitboxes on road hazards mean the player will frequently "just barely" dodge a car. This is intentional and is a major source of excitement. Do not "fix" this by making hitboxes match visuals.

3. **The multiplier exists to make retreating feel bad.** Without it, the optimal strategy is to inch forward and retreat constantly. The multiplier rewards bold, committed forward pushes.

4. **30 seconds per round is tight by design.** The timer creates urgency. Players should feel time pressure on their first few attempts. By Wave 3+, skilled players will finish with 10-15 seconds remaining. If playtesting reveals 30s is too punishing for first-time players, we can adjust -- but start tight and loosen, never the reverse.

5. **The grid is the game's backbone.** Every system (movement, hazards, scoring, collision) runs on the grid. If the grid system has bugs, everything breaks. It is the highest-priority implementation target.
