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
