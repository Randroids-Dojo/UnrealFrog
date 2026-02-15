# Level Designer Memory

*Persistent knowledge accumulated across sessions. First 200 lines loaded automatically.*

## Grid Coordinate Convention (Sprint 1 Implementation)
- `AFrogCharacter::GridToWorld()` maps grid (X,Y) to world (X*100, Y*100, 0)
- Column 0 at world X=0, column 12 at X=1200
- This differs from the gameplay spec which centers col 6 at world X=0
- Decision pending: keep current convention or refactor to spec

## Playfield Dimensions
- 13 columns x 15 rows = 1300 UU wide x 1500 UU deep
- World space: X=0..1300, Y=0..1500
- Center: (650, 750, 0)
- Frog spawns at grid (6, 0) = world (600, 0, 0)

## Zone Layout (Row -> Zone)
- Row 0: Start (safe, green)
- Rows 1-5: Road (gray, 5 lanes)
- Row 6: Median (safe, light green)
- Rows 7-12: River (blue, 6 lanes)
- Row 13: Goal approach (amber)
- Row 14: Goal with 5 home slots at columns 1,4,6,8,11

## Existing Systems
- LaneManager auto-configures via SetupDefaultLaneConfigs() when LaneConfigs is empty
- GameMode is already set as global default in DefaultEngine.ini
- Enhanced Input is configured with pure C++ actions
- Content directory has procedural audio (Content/Audio/SFX/*.wav)

## Sprint 2 Level Design Plan
- Recommended camera: position (650, 750, 1800), pitch -80, FOV 40-50
- Zone colors: green-start, gray-road, green-median, blue-river, amber-goal, red/green-goal-slots
- Suggested approach: auto-spawn everything from C++ (no .umap dependency)
- Lighting: one directional + one sky light, no post-processing

## Sprint 5 Spatial Observations (VFX + HUD)

### HomeSlotColumns Duplication Problem
HomeSlotColumns {1,4,6,8,11} hardcoded in THREE separate places:
1. `UnrealFrogGameMode.cpp` constructor (line 26)
2. `FroggerVFXManager::SpawnRoundCompleteCelebration()` (line 156) -- local `TArray<int32>`
3. `FroggerVFXManager::HandleHomeSlotFilled()` (line 205) -- local `TArray<int32>`
4. `GroundBuilder.cpp` constructor (line 24)
GridCellSize (100.0) also hardcoded in VFXManager as `Col * 100.0`.
If grid layout changes, 4 files must be updated manually.

### HUD Screen Space Budget
- Score panel: Y=10, top ~30px
- Timer bar: Y=40, height 8-14px (pulsing)
- Total HUD header: ~50px at top of screen
- At 720p (720px height), this is ~7% -- acceptable
- Title screen uses full-screen overlay with percentage-based vertical layout (30%, 45%, 55%, 85%)

### VFX World Positions
- Home slot VFX spawn at (Col*100, 14*100, Z=50)
- Hop dust spawns at frog location with Z=0
- Death puff spawns at frog location
- HomeSlotSparkle rises Z+60 over 1.0s duration
- VFX actors use no collision (ECollisionEnabled::NoCollision) -- good, won't interfere with gameplay

### VFX/HUD Z-ordering Risk
- Home slots at row 14 = Y=1400 in world space
- VFX sparkle rises to Z=110 (50 base + 60 rise)
- Camera is top-down at Z=2200, looking straight down
- HUD renders in screen space (Canvas), VFX in world space -- they cannot overlap in 3D
- HOWEVER: projection of row 14 world position onto screen may be near the top of the viewport, close to where HUD elements render

### Seam Matrix Level Design Gap
- Row 17 in seam-matrix.md: "GroundBuilder | LaneManager | Ground rows match lane config rows" is DEFERRED
- This is the spatial layout seam -- if GroundBuilder and LaneManager diverge, hazards float above wrong-colored ground
- Should be COVERED, not DEFERRED, for level design integrity
