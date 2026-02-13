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
- Enhanced Input is configured but no actions/mappings exist yet
- Content directory is effectively empty (no maps, no assets)

## Sprint 2 Level Design Plan
- Recommended camera: position (650, 750, 1800), pitch -80, FOV 40-50
- Zone colors: green-start, gray-road, green-median, blue-river, amber-goal, red/green-goal-slots
- Suggested approach: auto-spawn everything from C++ (no .umap dependency)
- Lighting: one directional + one sky light, no post-processing
