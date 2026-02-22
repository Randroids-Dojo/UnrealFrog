# Conversation: 2026-02-16

| Field | Value |
|-------|-------|
| Date | 2026-02-16 |
| Session ID | `486a1fce-113f-4722-9497-667c8c9ccddd` |
| Model | claude-haiku-4-5-20251001 |
| Tokens | ~1,381,434 input, ~85 output |
| Duration | 58s |

---

## User (04:18 UTC)

Explore the UnrealFrog codebase to understand frog movement mechanics for designing a safe-path calculation system. I need to know:

1. **FrogCharacter hop mechanics**: How does RequestHop work? What's HopDuration? How does the frog move (lerp? instant teleport?)? What blocks hops (bIsHopping guard)?

2. **Hazard movement**: How do hazards move? Are they strictly linear (constant velocity)? Do they wrap around? What happens at grid boundaries? Look at HazardBase.h/.cpp and any movement logic.

3. **Lane configuration**: What are the actual lane configs (speed, direction, hazard type, width, count per lane)? Look at LaneManager::SetupDefaultLaneConfigs().

4. **Timing**: What's the actual hop duration? How long between hops? Is there an input buffer? What's the timing contract between "request hop" and "frog arrives at destination"?

5. **Grid system**: GridCellSize, GridToWorld conversion, how positions are tracked (grid vs world). What's the frog's collision radius?

Read the relevant files in Source/UnrealFrog/Public/Core/ and Private/Core/ to answer these questions thoroughly.

## Assistant (04:18 UTC)

🤖

I'll systematically explore the UnrealFrog codebase to understand the frog movement mechanics and hazard behavior. Let me start by identifying the key files and then reading them.

- **Used Glob** for `**/Public/Core/*.h` in `.`

- **Used Glob** for `**/Private/Core/*.cpp` in `.`

- **Used Bash**: `find /Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog -name ...`

## Assistant (04:18 UTC)

Now let me read the key files to understand the movement mechanics:

- **Used Read** on `Source/UnrealFrog/Public/Core/FrogCharacter.h`

- **Used Read** on `Source/UnrealFrog/Public/Core/HazardBase.h`

- **Used Read** on `Source/UnrealFrog/Public/Core/LaneTypes.h`

## Assistant (04:18 UTC)

Now let me read the implementation files:

- **Used Read** on `Source/UnrealFrog/Private/Core/FrogCharacter.cpp`

- **Used Read** on `Source/UnrealFrog/Private/Core/HazardBase.cpp`

## Assistant (04:18 UTC)

Now let me read the LaneManager to understand lane configurations:

- **Used Read** on `Source/UnrealFrog/Private/Core/LaneManager.cpp` (limit=150)

- **Used Read** on `Source/UnrealFrog/Private/Core/LaneManager.cpp` (offset=150, limit=150)

## Assistant (04:18 UTC)

Now let me examine some tests to understand the timing and collision better:

- **Used Bash**: `grep -n "HopDuration\|GridCellSize\|HopElapsed\|COLLISION\|FindPlatform" /Use...`

- **Used Read** on `Source/UnrealFrog/Tests/SeamTest.cpp` (limit=200)

## Assistant (04:19 UTC)

Perfect! Now let me check the game mode state and any RPC/state query mechanisms:

- **Used Bash**: `grep -n "GetLaneHazardsJSON\|GetGameStateJSON\|RemoteControl" /Users/randroid...`

- **Used Read** on `Source/UnrealFrog/Public/Core/UnrealFrogGameMode.h` (limit=180)

## Assistant (04:19 UTC)

Excellent! Now I have all the information I need. Let me compile my findings into a comprehensive report.

## FROG MOVEMENT MECHANICS & SAFE-PATH SYSTEM ANALYSIS

Based on my exploration of the UnrealFrog codebase, here's the complete breakdown:

---

### 1. **FrogCharacter Hop Mechanics**

#### RequestHop Logic
- **File**: `FrogCharacter.cpp:121-154`
- **Guard conditions**: 
  - Rejects if `bIsDead` (returns early)
  - Rejects if `bIsHopping` BUT buffers input during final `InputBufferWindow` seconds
  - Rejects if target position invalid (outside grid)
  - Rejects if direction is zero-length

#### Movement Pattern: **LERP-based smooth interpolation**
- **File**: `FrogCharacter.cpp:70-99` (Tick logic)
- **Duration**: `HopDuration = 0.15f` (150ms) by default
- **XY Movement**: Linear interpolation from `HopStartLocation` to `HopEndLocation`
  ```cpp
  CurrentPos = FMath::Lerp(HopStartLocation, HopEndLocation, Alpha);
  ```
- **Z Movement**: Parabolic arc
  ```cpp
  ArcAlpha = 4.0f * Alpha * (1.0f - Alpha);  // 0 at endpoints, 1 at midpoint
  CurrentPos.Z = HopStartLocation.Z + HopArcHeight * ArcAlpha;
  ```
- **Arc height**: `HopArcHeight = 30.0f` UU (visual only, doesn't affect collision)

#### Input Buffer Mechanism
- **File**: `FrogCharacter.cpp:121-154`
- **Timing**: `InputBufferWindow = 0.08f` (80ms, ~53% of HopDuration)
- **When it activates**: During last 80ms of hop
  ```cpp
  float TimeRemaining = CurrentHopDuration - HopElapsed;
  if (TimeRemaining <= InputBufferWindow)
  {
    bHasBufferedInput = true;
    BufferedDirection = Direction;
  }
  ```
- **Processing**: After `FinishHop()`, automatically calls `RequestHop(BufferedDirection)` if buffered

#### Backward Hop Penalty
- **Duration multiplier**: `BackwardHopMultiplier = 1.0f` (tunable, not currently penalized)
- **File**: `FrogCharacter.cpp:181-189`

---

### 2. **Grid System**

#### Grid Constants
- **Dimensions**: 13 columns × 15 rows (grid indices 0-12 X, 0-14 Y)
- **Cell size**: `GridCellSize = 100.0f` UU
- **Total world space**: 1300 UU wide × 1500 UU tall
- **Origin**: (0, 0) is bottom-left corner

#### Position Conversions
- **GridToWorld** (line 156-162):
  ```cpp
  FVector GridToWorld(FIntPoint GridPos)
  {
    return FVector(GridPos.X * 100.0f, GridPos.Y * 100.0f, 0.0f);
  }
  ```
- **WorldToGrid** (line 165-170):
  ```cpp
  FIntPoint WorldToGrid(FVector WorldPos)
  {
    return FIntPoint(
      FMath::RoundToInt(WorldPos.X / 100.0f),
      FMath::RoundToInt(WorldPos.Y / 100.0f)
    );
  }
  ```

#### Frog Collision Size
- **File**: `FrogCharacter.cpp:18-19`
- **Capsule**: 34 UU radius, 44 UU height
- **Effective "touch zone"**: ~34 UU from center in XY plane
- **Mesh scale**: 0.8× sphere (40 UU radius visual)

#### River Detection
- **River rows**: 7-12 inclusive (configurable: `RiverRowMin=7`, `RiverRowMax=12`)
- **OnRiverRow()**: Returns true if `GridPosition.Y` is in [7, 12]

---

### 3. **Hazard Movement & Collision**

#### Movement Pattern: **Constant velocity, linear wrapping**
- **File**: `HazardBase.cpp:73-83`
- **Per-frame**: `Location.X += Speed * Direction * DeltaTime`
- **Direction**: `bMovesRight ? 1.0f : -1.0f`
- **No acceleration, no easing**

#### Wrapping Behavior
- **File**: `HazardBase.cpp:144-166`
- **Wrap boundaries**: 
  ```cpp
  float WrapMin = -GetWorldWidth();     // Hazard width past left edge
  float WrapMax = GridColumns * 100 + GetWorldWidth();  // Past right edge
  ```
- **Teleportation**: Hazards wrap from one side to the other instantly (no smooth wrapping)
- **Buffer**: One hazard width on each side to hide wrap

#### Hazard Width (in cells)
| Lane | Hazard Type | Width | Speed | Gap | Direction |
|------|------------|-------|-------|-----|-----------|
| 1 | Car | 1 | 150 | 3 | LEFT |
| 2 | Truck | 2 | 100 | 4 | RIGHT |
| 3 | Car | 1 | 200 | 2 | LEFT |
| 4 | Bus | 2 | 175 | 3 | RIGHT |
| 5 | Motorcycle | 1 | 250 | 2 | LEFT |

#### River (Rideable) Hazards
| Lane | Hazard Type | Width | Speed | Gap | Direction | Notes |
|------|------------|-------|-------|-----|-----------|-------|
| 7 | SmallLog | 2 | 100 | 2 | RIGHT | |
| 8 | TurtleGroup | 3 | 80 | 2 | LEFT | Submerges Wave 2+ |
| 9 | LargeLog | 4 | 120 | 2 | RIGHT | |
| 10 | SmallLog | 2 | 100 | 2 | LEFT | |
| 11 | TurtleGroup | 3 | 80 | 2 | RIGHT | Submerges Wave 2+ |
| 12 | LargeLog | 4 | 150 | 2 | LEFT | Fastest log |

#### Turtle Submerge Cycle (Wave 2+)
- **File**: `HazardBase.cpp:85-123`
- **Phases**:
  1. **Surface**: 4.0s (bIsSubmerged = false)
  2. **Warning**: 1.0s (bIsSubmerged = false, visual indicator)
  3. **Submerged**: 2.0s (bIsSubmerged = true, kills frog)
  4. Loops back to Surface
- **Total cycle**: 7 seconds

#### Collision Detection
- **Road hazards** (cars, trucks): Instant kill on overlap
- **River platforms** (logs, turtles): 
  - Mount on overlap (set `CurrentPlatform`)
  - Kill if submerged
- **Detection method**: 
  - Immediate overlap events on begin/end
  - **Synchronous platform re-detection after hop** (line 244): `FindPlatformAtCurrentPosition()` bypasses physics timing issues

#### Platform Riding
- **File**: `FrogCharacter.cpp:380-394` (UpdateRiding)
- **Mechanism**: Platform's velocity applied each frame to frog
  ```cpp
  FVector Location = GetActorLocation();
  float DeltaX = Platform->Speed * Direction * DeltaTime;
  Location.X += DeltaX;
  SetActorLocation(Location);
  ```
- **Frog grid position**: NOT updated while riding (only on hop landing)
- **Hop origin during riding**: Uses actual actor location, NOT grid-snapped position
  - **File**: `FrogCharacter.cpp:196-227` (StartHop)
  - If `CurrentPlatform.IsValid()`: use `GetActorLocation()` as origin
  - Otherwise: use `GridToWorld(GridPosition)`

---

### 4. **Timing Contract**

#### Hop Duration
- **Total duration**: 0.15s (150ms)
- **Input buffer opens**: After 0.07s (70ms, at 0.15 - 0.08)
- **Hop completion**: After 0.15s (full duration)
- **Buffered input processes**: Immediately after FinishHop

#### Frame-by-Frame Timeline
```
T=0.00s: RequestHop() called → StartHop() → bIsHopping=true, GridPosition updated
T=0.00-0.07s: Hop in progress, input rejected (not in buffer window)
T=0.07s: Input buffer window OPENS
T=0.07-0.15s: Input can be buffered
T=0.15s: FinishHop() → SetActorLocation(landing), FindPlatformAtCurrentPosition()
T=0.15s: If buffered input exists → RequestHop(bufferedDirection) auto-triggers
T=0.30s: Second hop completes (if buffered input was processed)
```

#### Death & Respawn Timing
- **RespawnDelay**: 1.0s
- **State flow**: Die → schedule respawn timer → Respawn clears `bIsDead`, resets position

#### Game State Transitions
- **SpawningDuration**: 1.0s (transition from Spawning → Playing)
- **DyingDuration**: 0.5s
- **RoundCompleteDuration**: 2.0s

---

### 5. **Safe-Path Calculation: Key Constraints**

#### Off-Screen Death
- **File**: `FrogCharacter.cpp:367-373`
- **Boundary**: `X < 0.0f || X >= GridColumns * GridCellSize`
- **Safe zone**: X ∈ [0, 1300)

#### River Death Conditions
1. On a river row AND
2. No valid rideable platform at current position OR
3. Platform is submerged

#### Platform Detection Algorithm
- **File**: `FrogCharacter.cpp:398-431` (FindPlatformAtCurrentPosition)
- **Method**: Direct world position check, NOT physics overlap
- **Check**: Frog center must be within hazard footprint
  ```cpp
  if (FMath::Abs(FrogPos.X - HazardPos.X) <= HalfWidth &&
      FMath::Abs(FrogPos.Y - HazardPos.Y) <= HalfCell)
  {
    CurrentPlatform = Hazard;
    return;
  }
  ```
- **HalfWidth**: `HazardWidthCells * GridCellSize / 2`
- **HalfCell**: `GridCellSize * 0.5` (100 * 0.5 = 50 UU)

#### For Safe-Path Calculation, You Need:

1. **Hazard positions at future timesteps** (via `GetLaneHazardsJSON()`)
   - Each hazard: `{x, y, width_cells, is_rideable, is_submerged}`
   - Compute position at time T: `pos_x(t) = initial_x + speed * direction * t`

2. **Collision radius**: ~34 UU (frogcapsule radius)

3. **Platform detection footprint**: Frog center must be within hazard ±HalfWidth (X) and ±50 UU (Y)

4. **Submerge schedule**: For turtles, compute submerge phase at time T based on 7-second cycle

5. **Grid boundaries**: Safe X ∈ [0, 1300), Safe Y ∈ [0, 1500)

---

### 6. **State Query via Remote Control API**

The game provides:
- **`GetGameStateJSON()`**: Score, lives, wave, timer, game state
- **`GetLaneHazardsJSON()`**: All hazard positions, widths, speeds, directions, submerge states
- **`GetObjectPath()` / `GetPawnPath()`**: Object paths for RC API

These are queryable via PlayUnreal at `http://localhost:30010` using `SearchActor` with blueprint-callable UFUNCTIONs.

---

### Summary for Safe-Path Algorithm

**Safe cell at grid position (X, Y, TimeT) is reachable iff:**

1. **Grid valid**: 0 ≤ X < 13, 0 ≤ Y < 15
2. **Lane type**: If Y ∈ [1-5] (road) → must have gap; if Y ∈ [7-12] (river) → must have rideable platform not submerged; else (safe/goal) → always safe
3. **Hazard gaps**: For road lanes, query all hazards in that row at time T, check if X falls in a gap
4. **Platform coverage**: For river lanes, query all turtles/logs at time T, check if X falls on a rideable (non-submerged) platform
5. **Off-screen**: World X position must stay in [0, 1300) range

The frog reaches grid (X, Y) after one hop at time T = 0.15s, so safe-path must predict hazard positions 150ms into the future.
