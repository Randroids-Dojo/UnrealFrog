---
name: asset-generation
description: Pipeline for generating, validating, and importing 3D models, textures, and audio assets
context: fork
---

## Asset Generation Pipeline

Generate → Validate → Convert → Import → Validate Again → Integrate

Every step must pass before proceeding to the next. No manual intervention.

## 3D Models

### Meshy AI (Primary)
- REST API, outputs FBX/GLB/OBJ
- PBR textures included
- Adjustable polygon count: 1K–300K faces
- Native UE plugin available
- Best for: characters, vehicles, props

```bash
# Example API call (requires MESHY_API_KEY)
curl -X POST https://api.meshy.ai/v2/text-to-3d \
  -H "Authorization: Bearer $MESHY_API_KEY" \
  -d '{"prompt": "low poly cartoon frog character", "topology": "quad", "target_polycount": 1500}'
```

### Blender Headless (Procedural Geometry)
- Full programmatic control via Python scripts
- Best for: lane markers, platforms, simple obstacles, tiling geometry

```bash
blender -b -P scripts/generate_road_tile.py -- --width 200 --length 100 --output road_tile.fbx
```

### Validation
- **trimesh**: Verify mesh integrity (watertight, no degenerate faces, correct normals)
- **Polycount check**: Must be within budget (see Art Director specifications)
- **Scale check**: Must match UE unit scale (1 unit = 1 cm)

## Textures

### Scenario API (Primary)
- Purpose-built for game development
- Generates complete PBR map sets: albedo, normal, roughness, metallic, height, AO
- REST API with style consistency features

### Poly Haven (CC0 Library)
- 100+ photoscanned PBR texture sets
- Public API, 8K+ resolution
- Best for: environmental surfaces, road, water, grass

### Procedural (Python)
- Pillow/NumPy for simple patterns
- Best for: noise textures, gradients, color fills for blockmesh

### Texture Standards
- Power-of-2 dimensions (256, 512, 1024, 2048)
- Albedo: RGB, no alpha unless transparency needed
- Normal: RGB, OpenGL format (convert from DirectX if needed)
- Roughness/Metallic: Grayscale
- Max resolution: 2048x2048 for props, 1024x1024 for tiling

## Audio

### ElevenLabs SFX V2
- Text-to-SFX generation
- 48kHz professional quality
- Supports looping and duration control (up to 30 seconds)
- Best for: realistic effects, ambient sounds

### rFXGen (Retro/Arcade)
- Single CLI executable (~1.5MB)
- Presets: coin, explosion, powerup, hit, jump
- WAV output
- Best for: retro arcade sound effects

```bash
# Generate a jump sound
rfxgen -t jump -o hop_sound.wav
```

### Audio Standards
- Source format: WAV 48kHz 16-bit
- Shipping format: OGG (compressed by UE on import)
- Naming: SW_ prefix for Sound Waves, SC_ for Sound Cues
- Import path: /Content/Audio/{Category}/

## UE Import Automation

```python
import unreal

def import_asset(filepath, destination, asset_type="mesh"):
    task = unreal.AssetImportTask()
    task.filename = filepath
    task.destination_path = destination
    task.automated = True
    task.replace_existing = True

    if asset_type == "mesh":
        task.options = unreal.FbxImportUI()
        task.options.import_materials = True
        task.options.import_textures = True

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return task.imported_object_paths
```

## Asset Budget (UnrealFrog)

| Category | Per-Asset Budget | Scene Budget |
|----------|-----------------|--------------|
| Frog character | 1000-2000 tris | — |
| Vehicle | 300-1000 tris | 10K total |
| Log/platform | 200-500 tris | 5K total |
| Environment prop | 100-500 tris | 20K total |
| Road/water tile | 50-200 tris | 5K total |
| **Total scene** | — | **< 100K tris** |
