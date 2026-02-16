# Art Director Memory

*Persistent knowledge accumulated across sessions. First 200 lines loaded automatically.*

## Sprint 2 Visual Plan (Whitebox)

### Shape Language
- Cubes = danger (road hazards)
- Cylinders = safe platforms (river objects, home slots)
- Sphere = player (frog) and turtle groups
- Planes = ground zones

### Color Palette (Sprint 2 Whitebox)
- Frog: Bright Green (0.1, 0.85, 0.2)
- Car: Red (0.9, 0.15, 0.1)
- Truck: Maroon (0.6, 0.1, 0.1)
- Bus: Yellow (0.95, 0.75, 0.1)
- Motorcycle: Orange (0.95, 0.5, 0.1)
- Small Log: Brown (0.55, 0.35, 0.15)
- Large Log: Dark Brown (0.4, 0.25, 0.1)
- Turtle Group: Dark Green (0.1, 0.5, 0.2)
- Home Slot (empty): Cyan (0.1, 0.9, 0.9)
- Home Slot (filled): Gray (0.3, 0.3, 0.3)
- Ground Start/Median: Green (#33CC33)
- Ground Road: Dark Gray (#404040)
- Ground River: Deep Blue (#1A3399)
- Ground Goal: Blue-Green (#1A6666)

### Existing Code State
- `AFrogCharacter` has `UStaticMeshComponent` MeshComponent (no mesh assigned)
- `AHazardBase` has `UStaticMeshComponent` MeshComponent (no mesh assigned)
- Both have collision components (capsule for frog, box for hazards)
- Engine primitives available at `/Engine/BasicShapes/Sphere`, `Cube`, `Cylinder`, `Plane`
- Use `ConstructorHelpers::FObjectFinder` to load engine primitives in constructors
- Use `UMaterialInstanceDynamic` for per-actor colors

### Grid Specs (from gameplay spec)
- 13 columns x 15 rows, 100 UU per cell
- Playfield: 1300 UU wide x 1500 UU deep
- Ground planes at Z=-1 to avoid Z-fighting

## Sprint 5 Visual Additions

### HUD Arcade Color Scheme
- Score text: Green FColor(0, 255, 100, 255) -- classic arcade readout
- High score text: Yellow FColor(255, 255, 0, 255) -- top-center
- Lives/wave text: Red-tinted FColor(255, 100, 100, 255) -- top-right
- Timer background: Near-black FColor(20, 20, 20, 220)
- Timer foreground: Green->Red gradient based on TimerPercent
- Timer pulse: Height oscillation (8-14px) + red flash when <16.7% remaining
- Overlay backgrounds: Semi-transparent black FColor(0,0,0,180)
- Score pops: Yellow for normal, White for bonus (>100 pts), 1.5s rise+fade
- Wave announcement: Yellow, fade-in 0.3s -> hold 1.0s -> fade-out 0.7s

### Title Screen
- Full-screen dark overlay FColor(10, 10, 20, 240)
- "UNREAL FROG" title: Color pulse green(0,255,100)<->yellow(255,255,0) at 1.5Hz
- "PRESS START" blinks on/off at 2Hz, white
- "HI-SCORE: N" shown in yellow when > 0
- "A MOB PROGRAMMING PRODUCTION" credits: Dim gray FColor(100,100,100,200) at bottom

### VFX System (UFroggerVFXManager)
- Death puff sphere: red(1,0.2,0.2) squish/timeout, blue(0.2,0.4,1) splash, yellow(1,1,0.2) offscreen
- Hop dust: 3 cubes, brown(0.6,0.5,0.3), 0.2s pop, triangular spread 20UU
- Home slot sparkle: 4 spheres, gold(1,0.85,0), 1.0s rise at 60 UU/s
- Round complete: Sparkles at all 5 home slot positions
- Animation: Scale-only (StartScale->EndScale over Duration). No rotation, no alpha fade.
- MaxActiveVFX = 10 (oldest destroyed when cap reached)
- 5.0s auto-lifespan failsafe via SetLifeSpan
- bDisabled flag for CI/headless testing

### Visual Concerns Noted
- All rendering uses Canvas API with GEngine->GetMediumFont/GetLargeFont -- no custom fonts
- VFX are scale-only animation (no rotation, no alpha fade on geometry)
- Timer bar height oscillation is unsmoothened (raw sine)
- No consistent VFX shape language (sphere for death + sparkle, cube for dust)
- Score pop text uses DrawText with no outline/shadow for readability
- FlatColorMaterial.h still used -- M_FlatColor.uasset still deferred

### Sprint 6 Recommendations (from Art Director)
- Custom pixel/arcade font for HUD (elevates entire visual identity)
- VFX rotation animation (spinning sparkles, tumbling dust)
- Screen flash on death (full-screen red overlay, 0.1s) -- DONE (Commit 4)
- Scanline overlay effect for CRT/arcade feel
- Alpha fade on VFX geometry (not just scale)
- Consistent VFX shape language: all spheres, or all cubes, not mixed

### Sprint 6 Commit 4: HUD Fixes
- **Score pop proportional positioning**: Replaced hardcoded `FVector2D(160.0f, 10.0f)` with `FVector2D(20.0f + ScoreText.Len() * 10.0f, 10.0f)` -- pop appears right after the score text regardless of digit count
- **Death flash overlay**: Full-screen red `FColor(255, 0, 0, Alpha)` with `SE_BLEND_Translucent`, triggered at alpha 0.5 on `EGameState::Dying`, decays at 1.67/s (~0.3s total). Drawn before score panel so it underlays HUD text.
- **TickDeathFlash()**: Public method for test-driven decay verification without Canvas dependency
- **DrawDeathFlash()**: Private Canvas drawing helper, full-screen FCanvasTileItem
- **Tests added**: `FHUD_DeathFlashActivates` (trigger logic), `FHUD_DeathFlashDecays` (decay math via TickDeathFlash)

## Sprint 11 Retro: Three.js to UE Pipeline Research

### WebFrogger Asset Inventory
- 3 geometry types only: BoxGeometry (17), CylinderGeometry (8), SphereGeometry (7)
- All materials are MeshLambertMaterial with a single color (21 unique colors)
- 10 model factories: Frog, Car, Truck, Bus, RaceCar, Log, TurtleGroup, LilyPad, SmallFrog, Hedge
- Each model = THREE.Group with 3-12 positioned primitives
- Scale: CELL = 1.0 in Three.js vs 100 UU in UnrealFrog (100x multiplier)

### Pipeline Decision: Direct Translation (Approach 1)
- No file format conversion needed for WebFrogger-level complexity
- Three.js primitives map 1:1 to UE engine primitives (Cube, Sphere, Cylinder)
- MeshLambertMaterial({color}) -> FlatColorMaterial + SetVectorParameterValue("Color")
- THREE.Group -> AActor with UStaticMeshComponent children at SetRelativeLocation offsets
- Produces zero binary assets -- pure C++ actor definitions

### UE 5.7 glTF Capabilities (for future reference)
- Interchange plugin: built-in, enabled by default, full glTF import via UInterchangeGLTFTranslator
- Supports: static meshes, materials, textures, skeletal meshes, animations
- GLTFExporter plugin: round-trip export (UE -> glTF)
- GLTFCore parser: scene graph, mesh data, material slots
- Automated import via UAssetImportTask + PythonScriptPlugin

### WebFrogger Color Palette (hex, for direct porting)
- frog: 0x22cc22, frogBelly: 0x88ee44, frogEye: 0xffffff, frogPupil: 0x111111
- car1: 0xff3333, car2: 0x3366ff, car3: 0xffaa00
- truck: 0x884488, bus: 0xffcc00, raceCar: 0xff00ff
- log: 0x8B4513, logLight: 0xa0522d
- turtle: 0x006633, turtleShell: 0x228844
- lilyPad: 0x117711, lilyFlower: 0xff69b4
- road: 0x333333, roadLine: 0xffff00
- grass: 0x226622, median: 0x337733
- water: 0x1a3a5c, sidewalk: 0x777777, hedge: 0x0a440a
