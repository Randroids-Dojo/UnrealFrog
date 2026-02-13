"""
Create the minimal FroggerMain map for UnrealFrog.

Run via UnrealEditor-Cmd:
  "/Users/Shared/Epic Games/UE_5.7/Engine/Binaries/Mac/UnrealEditor-Cmd" \
    "/Users/randroid/Documents/Dev/Unreal/UnrealFrog/UnrealFrog.uproject" \
    -ExecutePythonScript="/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Tools/CreateMap/create_frog_map.py" \
    -unattended -nopause

The map is just an empty container. All gameplay content is auto-spawned
by the C++ GameMode (FrogCharacter, LaneManager, GroundBuilder, Camera, HUD).
"""

import unreal

MAP_PATH = "/Game/Maps/FroggerMain"

# Create a new empty level
success = unreal.EditorLevelLibrary.new_level(MAP_PATH)

if success:
    # The GameMode is set globally in DefaultEngine.ini, so no per-level override needed.
    # All actors (frog, lanes, ground, camera) are spawned in GameMode::BeginPlay.

    # Save the level
    unreal.EditorLevelLibrary.save_current_level()

    unreal.log(f"Created map: {MAP_PATH}")
else:
    unreal.log_error(f"Failed to create map: {MAP_PATH}")
