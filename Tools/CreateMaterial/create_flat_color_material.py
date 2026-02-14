"""
Create M_FlatColor — a simple tintable material with a "Color" VectorParameter.

Run via:
  UnrealEditor-Cmd <project> -ExecutePythonScript=Tools/CreateMaterial/create_flat_color_material.py

This creates /Game/Materials/M_FlatColor with a single VectorParameter ("Color")
wired to BaseColor. All gameplay actors use this material for colored placeholders.
"""

import unreal

ASSET_PATH = "/Game/Materials"
ASSET_NAME = "M_FlatColor"
FULL_PATH = f"{ASSET_PATH}/{ASSET_NAME}"

# Check if material already exists
if unreal.EditorAssetLibrary.does_asset_exist(FULL_PATH):
    unreal.log(f"Material already exists at {FULL_PATH}, skipping creation.")
else:
    # Create the material asset
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    material = asset_tools.create_asset(
        ASSET_NAME, ASSET_PATH,
        unreal.Material, unreal.MaterialFactoryNew()
    )

    # Create a VectorParameter expression named "Color" with white default
    color_param = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionVectorParameter, -300, 0
    )
    color_param.parameter_name = "Color"
    color_param.default_value = unreal.LinearColor(1.0, 1.0, 1.0, 1.0)

    # Connect the parameter output to the material's BaseColor input
    unreal.MaterialEditingLibrary.connect_material_property(
        color_param, "", unreal.MaterialProperty.MP_BASE_COLOR
    )

    # Recompile and save
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_asset(FULL_PATH)

    unreal.log(f"Created material: {FULL_PATH}")
