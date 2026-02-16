# Conversation: 2026-02-13

| Field | Value |
|-------|-------|
| Date | 2026-02-13 |
| Session ID | `2676fd16-2f8c-492a-af3b-1239ff04855a` |
| Model | claude-opus-4-6 |
| Tokens | ~2,387,478 input, ~1,071 output |
| Duration | 9m 34s |

---

## User (05:03 UTC)

Research how to programmatically create Unreal Engine 5 map files (.umap) and binary assets (.uasset) WITHOUT opening the editor GUI. We need agents (CLI-based, no GUI access) to create maps and Enhanced Input assets for a UE 5.7 project.

Investigate these approaches thoroughly:

## Approach 1: UE5 Commandlets
- Can we write a custom commandlet that creates a UWorld, adds actors, and saves to .umap?
- Research `UEditorLoadingAndSavingUtils::SaveMap()`, `FEditorFileUtils::SaveLevel()`, `UWorld::SaveWorld()`
- Can commandlets access the full editor API (actor spawning, component setup)?
- How to run: `UnrealEditor-Cmd ProjectName -run=MyCommandlet`
- Look for examples of commandlets that create/modify maps

## Approach 2: Python Editor Scripting (unreal module)
- UE5 has a Python scripting plugin that can run headlessly
- `import unreal` gives access to editor APIs
- Can we create maps, place actors, set world settings, save?
- Research: `unreal.EditorLevelLibrary`, `unreal.EditorAssetLibrary`, `unreal.AssetToolsHelpers`
- How to run: `UnrealEditor-Cmd ProjectName -ExecutePythonScript="script.py"`
- Can this create InputAction and InputMappingContext assets?

## Approach 3: Unreal Automation/Remote Control
- Remote Control API (HTTP) - can it create assets?
- Automation Driver - designed for UI testing, can it create maps?
- Editor Utility Widgets/Blueprints run from commandline?

## Approach 4: Editor Plugin with REST API
- Build a custom editor plugin that exposes map creation via HTTP
- Agents call the API to create maps, place actors, save
- More complex but most flexible

## Approach 5: Computer Use MCP
- Use an AI vision model to control the editor GUI
- Click through menus, place actors visually
- Research if Anthropic's computer use API or similar MCP servers exist for this
- Pros/cons vs programmatic approaches

## Approach 6: Minimal C++ Auto-Setup (No Map Needed)
- GameMode spawns everything in BeginPlay
- An empty/default map works
- No binary assets needed at all
- What are the limitations? (PlayerStart, World Settings overrides, etc.)

For each approach, determine:
1. Is it technically feasible with UE 5.7?
2. What are the prerequisites (plugins to enable, modules to add)?
3. How reliable is it? Will it work headlessly from a terminal?
4. How much setup work is needed?
5. Does it solve BOTH problems (maps AND Enhanced Input assets)?

Search the web for UE5 documentation, forum posts, and examples. Look specifically for:
- "UE5 commandlet create map"
- "UE5 python scripting create level"
- "Unreal Engine headless asset creation"
- "UE5 create InputAction programmatically"
- "Unreal Engine 5 editor scripting automation"

Provide a clear recommendation with justification.

## Assistant (05:03 UTC)

🤖️

I'll conduct a thorough investigation of all six approaches for programmatically creating UE5 map and asset files without the editor GUI. Let me research each one in parallel.

- **Used WebSearch**: `UE5 commandlet create map level programmatically save world`

- **Used WebSearch**: `UE5 python scripting create level map headless unreal module EditorLevelLibrary`

- **Used WebSearch**: `UE5 create InputAction InputMappingContext asset programmatically C++ Enhanced Input`

- **Used WebSearch**: `Unreal Engine 5 create Enhanced Input assets programmatically NewObject UInputAction UInputMappingContext runtime`

- **Used WebSearch**: `UE5 commandlet create save uasset package UPackage SavePackage programmatically`

- **Used WebSearch**: `UE5 python scripting create save asset headless -ExecutePythonScript unreal.AssetTools create_asset`

- **Used WebSearch**: `Unreal Engine 5 Remote Control API create asset REST HTTP headless`

- **Used WebSearch**: `Anthropic computer use MCP Unreal Engine GUI automation AI control desktop`

- **Used WebSearch**: `UE5 GameMode spawn actors BeginPlay no map needed default level PlayerStart`

- **Used WebSearch**: `"UE5" "Enhanced Input" "NewObject" "UInputAction" create runtime C++ without editor`

- **Used WebSearch**: `UE5 Enhanced Input pure C++ NewObject UInputAction UInputMappingContext runtime no asset code example`

- **Used WebFetch** on `https://codingbabble.com/posts/ue5-enhanced-input-in-pure-cpp/`

- **Used WebFetch** on `https://forums.unrealengine.com/t/procedurally-create-a-map-in-c/467959`

- **Used WebFetch** on `https://forums.unrealengine.com/t/creating-a-new-level-inside-a-commandlet/118555`

- **Used WebFetch** on `https://georgy.dev/posts/save-uobject-to-package/`

- **Used WebSearch**: `UE5 commandlet create new world level from scratch UWorld::CreateWorld save example`

- **Used WebFetch** on `https://ikrima.dev/ue4guide/engine-programming/uworld-creation-flow/`

- **Used WebSearch**: `unreal-mcp plugin Claude MCP server create map spawn actors headless capabilities`

- **Used WebSearch**: `UE5 python script create InputAction InputMappingContext Enhanced Input asset unreal module`

- **Used WebSearch**: `UE5 python unreal.AssetToolsHelpers create_asset factory InputActionFactory headless scripting`

- **Used WebSearch**: `UE5 UnrealEditor-Cmd commandlet headless mode -nullrhi create save map level working example 2024 2025`

- **Used WebFetch** on `https://slowburn.dev/blog/headless-fun-unreal-engine/`

- **Used WebSearch**: `UE5 commandlet editor API available GEditor spawn actor create world commandlet context 2024`

- **Used WebFetch** on `https://www.oneoddsock.com/2020/07/08/ue4-how-to-write-a-commandlet/`

- **Used WebSearch**: `UE5 "UEditorLoadingAndSavingUtils" "NewBlankMap" commandlet create new empty map save`

- **Used WebFetch** on `https://forums.unrealengine.com/t/commandlet-savemap-crash/453547`

- **Used WebSearch**: `"Unreal Engine" "computer use" MCP "Claude" screenshot control editor macOS 2024 2025`

- **Used WebSearch**: `Anthropic "computer use" API beta macOS desktop automation control GUI applications 2025 2026`

- **Used WebSearch**: `UE5 GameMode override InitGame BeginPlay spawn all actors programmatically without PlayerStart empty map pure C++`

- **Used WebSearch**: `UE5 default map project settings without editor set DefaultMap ini config GameMapsSettings`

## Assistant (05:08 UTC)

Now let me check the current project structure and config files to understand the existing setup.

- **Used Glob** for `**/*.ini` in `Config`

- **Used Read** on `Config/DefaultEngine.ini`

- **Used Glob** for `**/*.h` in `Source`

- **Used Read** on `Source/UnrealFrog/Public/Core/UnrealFrogGameMode.h`

- **Used Read** on `Source/UnrealFrog/Public/Core/FrogCharacter.h`

- **Used Read** on `Source/UnrealFrog/UnrealFrog.Build.cs`

## Assistant (05:09 UTC)

Now let me check if there are any existing map files:

- **Used Glob** for `**/*.umap` in `.`

- **Used Glob** for `**/*.uasset` in `.`

- **Used WebSearch**: `UE5 create UInputAction UInputMappingContext pure C++ code runtime no data asset NewObject example working`

- **Used WebFetch** on `https://unrealcommunity.wiki/using-the-enhancedinput-system-in-c++-74b72b`

- **Used WebSearch**: `UE5 python unreal.EditorLevelLibrary new_level save_current_level EditorAssetLibrary headless commandlet create map 2024`

- **Used WebFetch** on `https://dev.epicgames.com/documentation/en-us/unreal-engine/scripting-the-unreal-editor-using-python`

- **Used WebSearch**: `chongdashu unreal-mcp capabilities create level map save asset spawn actor full list tools`

- **Used WebFetch** on `https://github.com/chongdashu/unreal-mcp/blob/main/Docs/README.md`

- **Used WebFetch** on `https://github.com/chongdashu/unreal-mcp/blob/main/Python/README.md`

- **Used WebSearch**: `UE5 "FEditorFileUtils" OR "UEditorLoadingAndSavingUtils" NewMapTemplateAsset create empty map programmatically commandlet`

- **Used WebFetch** on `https://forums.unrealengine.com/t/how-to-save-umap-without-ue4editor/437519`

- **Used WebSearch**: `UE5 "UWorld::CreateWorld" "SavePackage" commandlet create save new map level working code example complete`

- **Used WebFetch** on `https://forums.unrealengine.com/t/creating-world-level-by-c/424850`

## Assistant (05:11 UTC)

Now let me do a final search specifically about the feasibility of Approach 6 (the C++ auto-setup approach) and any limitations.

- **Used WebSearch**: `UE5 empty default map GameMode spawns everything BeginPlay no content browser minimal map required`

## Assistant (05:12 UTC)

Now I have comprehensive research. Let me compile the findings into a thorough report.

---

🤖️

# Research Report: Programmatic UE5 Map & Asset Creation Without Editor GUI

## Executive Summary

After extensive research across documentation, forums, community wikis, and blog posts, I've evaluated all six approaches. **The clear recommendation is a hybrid of Approach 6 (C++ Auto-Setup) as the primary strategy, with Approach 2 (Python Editor Scripting) as the fallback for creating the initial minimal map file.** Here's the full analysis.

---

## Approach 1: Custom Commandlets (C++)

### Feasibility: MODERATE -- Works but fragile

**How it works:** A commandlet is a `UCommandlet` subclass that runs via `UnrealEditor-Cmd` with the `-run=` flag. Commandlets have access to editor APIs including `GEditor`, the Asset Registry, and `UPackage::SavePackage`.

**Running a commandlet:**
```
"/Users/Shared/Epic Games/UE_5.7/Engine/Binaries/Mac/UnrealEditor-Cmd" \
  "/Users/randroid/Documents/Dev/Unreal/UnrealFrog/UnrealFrog.uproject" \
  -run=MyCommandlet
```

**Map creation pattern (theoretical):**
```cpp
UCLASS()
class UCreateMapCommandlet : public UCommandlet
{
    GENERATED_BODY()
    virtual int32 Main(const FString& Params) override;
};

int32 UCreateMapCommandlet::Main(const FString& Params)
{
    // Create a new blank map
    UWorld* World = UEditorLoadingAndSavingUtils::NewBlankMap(false);
    
    // Spawn actors into it
    FActorSpawnParameters SpawnParams;
    World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), 
        FTransform::Identity, SpawnParams);
    
    // Save
    UEditorLoadingAndSavingUtils::SaveMap(World, TEXT("/Game/Maps/FrogMap"));
    return 0;
}
```

**Saving a generic asset (.uasset):**
```cpp
UPackage* Package = CreatePackage(TEXT("/Game/Input/IA_Hop"));
UInputAction* Action = NewObject<UInputAction>(Package, TEXT("IA_Hop"),
    RF_Public | RF_Standalone);
Action->ValueType = EInputActionValueType::Boolean;

FString Filename = FPackageName::LongPackageNameToFilename(
    TEXT("/Game/Input/IA_Hop"), 
    FPackageName::GetAssetPackageExtension());
FSavePackageArgs SaveArgs;
SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
UPackage::SavePackage(Package, Action, *Filename, SaveArgs);
```

**Verdict:**
| Criterion | Assessment |
|-----------|-----------|
| Technically feasible | Yes, but `NewBlankMap` has documented crash issues |
| Prerequisites | Requires `UnrealEd` module dependency (editor-only) |
| Headless reliability | Partial -- commandlets run in editor context without GUI, but some APIs expect editor subsystems |
| Setup work | High -- must write commandlet class, add to build, compile |
| Solves maps? | Yes, with caveats (crashes reported with actors in maps) |
| Solves Enhanced Input assets? | Yes, via `NewObject` + `SavePackage` |

**Key gotchas:**
- `UEditorLoadingAndSavingUtils::NewBlankMap()` has documented crash reports ([Forum: Commandlet savemap crash](https://forums.unrealengine.com/t/commandlet-savemap-crash/453547))
- `UEditorLoadingAndSavingUtils::SaveMap()` crashes when maps contain actors ([Forum: Procedurally create a map](https://forums.unrealengine.com/t/procedurally-create-a-map-in-c/467959))
- The `FEditorFileUtils` approach requires `UnrealEd` module which causes build issues in game targets ([Forum: Creating World/Level by C++](https://forums.unrealengine.com/t/creating-world-level-by-c/424850))

**Sources:**
- [How To Write a Commandlet (Brent Scriver)](https://www.oneoddsock.com/2020/07/08/ue4-how-to-write-a-commandlet/)
- [Save UObject to Package (Georgy's Blog)](https://georgy.dev/posts/save-uobject-to-package/)
- [UPackage::SavePackage API Reference](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/CoreUObject/UPackage/SavePackage)
- [UWorld Creation Flow (ikrima)](https://ikrima.dev/ue4guide/engine-programming/uworld-creation-flow/)

---

## Approach 2: Python Editor Scripting

### Feasibility: HIGH -- Best approach for asset creation

**How it works:** UE5's Python scripting plugin exposes the `unreal` module with access to editor APIs. Scripts can be run headlessly via the `pythonscript` commandlet.

**Running headlessly (two modes):**
```bash
# Mode 1: Full editor context (slower, loads default level)
"/Users/Shared/Epic Games/UE_5.7/Engine/Binaries/Mac/UnrealEditor-Cmd" \
  "/Users/randroid/Documents/Dev/Unreal/UnrealFrog/UnrealFrog.uproject" \
  -ExecutePythonScript="/path/to/create_assets.py"

# Mode 2: Commandlet (faster, headless, no auto level load)
"/Users/Shared/Epic Games/UE_5.7/Engine/Binaries/Mac/UnrealEditor-Cmd" \
  "/Users/randroid/Documents/Dev/Unreal/UnrealFrog/UnrealFrog.uproject" \
  -run=pythonscript -script="/path/to/create_assets.py"
```

**Creating a map:**
```python
import unreal

# Create new level
editor_level = unreal.EditorLevelLibrary()
editor_level.new_level('/Game/Maps/FrogMap')

# Spawn actors
actor_location = unreal.Vector(0, 0, 0)
actor_rotation = unreal.Rotator(0, 0, 0)
player_start = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.PlayerStart, actor_location, actor_rotation)

# Save
editor_level.save_current_level()
```

**Creating Enhanced Input assets:**
```python
import unreal

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

# Create InputAction
factory = unreal.InputActionFactory()  # if exposed
ia_hop = asset_tools.create_asset('IA_Hop', '/Game/Input', 
    unreal.InputAction, factory)
ia_hop.set_editor_property('value_type', 
    unreal.InputActionValueType.BOOLEAN)

# Save
unreal.EditorAssetLibrary.save_asset('/Game/Input/IA_Hop')
```

**Prerequisites:**
1. Enable "Python Editor Script Plugin" in the `.uproject` file:
```json
{
    "Plugins": [
        { "Name": "PythonScriptPlugin", "Enabled": true },
        { "Name": "EditorScriptingUtilities", "Enabled": true }
    ]
}
```

**Verdict:**
| Criterion | Assessment |
|-----------|-----------|
| Technically feasible | Yes |
| Prerequisites | Two plugins to enable in `.uproject` |
| Headless reliability | Good with `-ExecutePythonScript` mode; commandlet mode has caveats (no auto level loading) |
| Setup work | Low -- just Python scripts, no compilation |
| Solves maps? | Yes (`EditorLevelLibrary.new_level()`) |
| Solves Enhanced Input assets? | Likely yes via `AssetTools.create_asset()` with appropriate factory |

**Key limitations:**
- Python environment is **editor-only** -- not available at runtime ([Official docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/scripting-the-unreal-editor-using-python))
- Marked as "Experimental" by Epic
- Commandlet mode does not auto-load levels (must load manually first)
- Whether `InputActionFactory` is exposed to Python depends on UE 5.7's specific bindings (needs verification)

**Sources:**
- [Scripting the Unreal Editor Using Python (Official)](https://dev.epicgames.com/documentation/en-us/unreal-engine/scripting-the-unreal-editor-using-python)
- [EditorLevelLibrary Python API](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/EditorLevelLibrary?application_version=5.6)
- [AssetTools Python API](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/AssetTools?application_version=5.3)

---

## Approach 3: Remote Control API / Automation Driver

### Feasibility: LOW -- Requires running editor

**How it works:** UE5's Remote Control API runs an HTTP server on port 30010 inside the editor, allowing external tools to read/write properties and call `BlueprintCallable` functions.

**Capabilities:**
- Read/write object properties: `PUT /remote/object/property`
- Call functions: `PUT /remote/object/call`
- Access actors in the current level

**Critical limitation:** The Remote Control API can only interact with **objects already loaded in memory**. It cannot:
- Create new packages or assets
- Create new maps
- Spawn actors that don't exist yet (no direct spawn endpoint)
- Save files to disk

**Verdict:**
| Criterion | Assessment |
|-----------|-----------|
| Technically feasible | No for asset creation |
| Prerequisites | Remote Control plugin enabled, editor running with a map loaded |
| Headless reliability | Requires editor GUI running |
| Setup work | N/A |
| Solves maps? | No |
| Solves Enhanced Input assets? | No |

**Sources:**
- [Remote Control API HTTP Reference (Official)](https://dev.epicgames.com/documentation/en-us/unreal-engine/remote-control-api-http-reference-for-unreal-engine)
- [Remote Control for Unreal Engine (Official)](https://dev.epicgames.com/documentation/en-us/unreal-engine/remote-control-for-unreal-engine)

---

## Approach 4: Editor Plugin with REST API

### Feasibility: HIGH but OVERKILL

**How it works:** Build a custom editor plugin that registers HTTP endpoints, giving agents full access to editor APIs (asset creation, map saving, actor spawning) via REST calls.

This is essentially a custom version of what the MCP servers (chongdashu/unreal-mcp, kvick-games/UnrealMCP) do. These existing MCP servers already support:
- Creating/deleting actors
- Setting transforms and properties
- Blueprint class creation
- Running Python scripts within the editor

**Critical limitation:** Like Approach 3, this **requires the editor to be running with a GUI**. The MCP servers communicate with an editor plugin via TCP/HTTP, meaning the editor must be open.

**Verdict:**
| Criterion | Assessment |
|-----------|-----------|
| Technically feasible | Yes |
| Prerequisites | Editor running with GUI, plugin installed |
| Headless reliability | No -- requires editor GUI |
| Setup work | Very high for custom; moderate to use existing MCP servers |
| Solves maps? | Yes (via editor APIs) |
| Solves Enhanced Input assets? | Yes (via editor APIs) |

**Sources:**
- [chongdashu/unreal-mcp (GitHub)](https://github.com/chongdashu/unreal-mcp)
- [kvick-games/UnrealMCP (GitHub)](https://github.com/kvick-games/UnrealMCP)
- [flopperam/unreal-engine-mcp (GitHub)](https://github.com/flopperam/unreal-engine-mcp)

---

## Approach 5: Computer Use MCP

### Feasibility: MODERATE -- Works but slow and brittle

**How it works:** Anthropic's Computer Use API (beta) allows Claude to control a desktop by taking screenshots, moving cursors, clicking buttons, and typing text. This could be used to control the Unreal Editor GUI.

**Current status (2026):**
- Available in beta via the API with header `anthropic-beta: computer-use-2025-01-24`
- Claude Opus 4.6 supports `computer_20251124` tool version with zoom actions
- Works on macOS via community implementations ([GitHub: Anthropic-Computer-Use-MacOS](https://github.com/newideas99/Anthropic-Computer-Use-MacOS))

**What it could do:**
- Open Unreal Editor
- Create maps via File > New Level
- Create InputAction/InputMappingContext via Content Browser right-click menus
- Place actors via drag-and-drop or viewport clicking
- Save via Ctrl+S

**Verdict:**
| Criterion | Assessment |
|-----------|-----------|
| Technically feasible | Yes |
| Prerequisites | Editor GUI must be visible on screen, Computer Use API access, macOS accessibility permissions |
| Headless reliability | NO -- fundamentally requires a visible GUI |
| Setup work | Moderate (Computer Use orchestration code) |
| Solves maps? | Yes |
| Solves Enhanced Input assets? | Yes |

**Pros:** Can do literally anything a human can do in the editor.
**Cons:** Extremely slow (screenshot-action-screenshot loop), brittle (UI changes break it), expensive (every action requires an API call with image), cannot run headlessly.

**Sources:**
- [Anthropic Computer Use Docs](https://docs.anthropic.com/en/docs/agents-and-tools/computer-use)
- [Control Your Mac with Claude (APIdog)](https://apidog.com/blog/claude-computer-use/)

---

## Approach 6: Minimal C++ Auto-Setup (No Map Needed)

### Feasibility: HIGHEST -- Recommended primary approach

**How it works:** Instead of creating binary assets, the GameMode's `BeginPlay()` / `InitGame()` spawns everything programmatically. Enhanced Input actions and mapping contexts are created at runtime using `NewObject<>()`.

**For the map problem:** UE5 needs *some* map to load, but it can be a completely empty one. The `DefaultEngine.ini` already references `/Game/Maps/MainMenu` -- we just need that map to exist (even as a blank file). The GameMode, configured in `DefaultEngine.ini`, does all the work.

**For the PlayerStart problem:** If no `PlayerStart` exists, the player spawns at `(0, 0, 0)`. You can override `ChoosePlayerStart()` or `FindPlayerStart()` in the GameMode to return any transform you want.

**For Enhanced Input -- this is the key insight:** You do NOT need `.uasset` files for Enhanced Input. The [Coding Babble tutorial](https://codingbabble.com/posts/ue5-enhanced-input-in-pure-cpp/) demonstrates a complete working pattern:

```cpp
// IA_Hop.h -- Input Action subclass, NO asset needed
UCLASS()
class UIA_Hop : public UInputAction
{
    GENERATED_BODY()
public:
    UIA_Hop()
    {
        ValueType = EInputActionValueType::Boolean;
    }
};

// IMC_Frog.h -- Input Mapping Context subclass, NO asset needed
UCLASS()
class UIMC_Frog : public UInputMappingContext
{
    GENERATED_BODY()
private:
    UIA_Hop* HopForward;
    UIA_Hop* HopBack;
    UIA_Hop* HopLeft;
    UIA_Hop* HopRight;
public:
    UIMC_Frog()
    {
        HopForward = NewObject<UIA_Hop>();
        HopBack = NewObject<UIA_Hop>();
        HopLeft = NewObject<UIA_Hop>();
        HopRight = NewObject<UIA_Hop>();
        
        Mappings.Add(FEnhancedActionKeyMapping(HopForward, EKeys::W));
        Mappings.Add(FEnhancedActionKeyMapping(HopForward, EKeys::Up));
        Mappings.Add(FEnhancedActionKeyMapping(HopBack, EKeys::S));
        Mappings.Add(FEnhancedActionKeyMapping(HopBack, EKeys::Down));
        Mappings.Add(FEnhancedActionKeyMapping(HopLeft, EKeys::A));
        Mappings.Add(FEnhancedActionKeyMapping(HopLeft, EKeys::Left));
        Mappings.Add(FEnhancedActionKeyMapping(HopRight, EKeys::D));
        Mappings.Add(FEnhancedActionKeyMapping(HopRight, EKeys::Right));
    }
};

// In PlayerController::SetupPlayerInputComponent()
void AFrogPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    
    auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
        GetLocalPlayer());
    
    UIMC_Frog* FrogMappings = NewObject<UIMC_Frog>();
    Subsystem->AddMappingContext(FrogMappings, 0);
    
    auto* EIC = Cast<UEnhancedInputComponent>(InputComponent);
    // CRITICAL: bind the action from the mapping's own Mappings array
    EIC->BindAction(FrogMappings->Mappings[0].Action, ETriggerEvent::Triggered, 
        this, &AFrogPlayerController::HandleHopForward);
}
```

**For the minimal map problem:** You need ONE map file, but it can be created via a one-time Python script or commandlet, then committed. The map itself is empty -- all gameplay content is spawned by C++.

**Verdict:**
| Criterion | Assessment |
|-----------|-----------|
| Technically feasible | Yes, fully proven pattern |
| Prerequisites | None beyond existing project setup |
| Headless reliability | 100% -- all code, no binary assets needed for gameplay |
| Setup work | Low -- just C++ classes following existing patterns |
| Solves maps? | Partially -- still need one minimal .umap, but content is code-driven |
| Solves Enhanced Input assets? | YES, completely -- no .uasset files needed |

**Limitations:**
- World Settings overrides (lighting, post-process, etc.) would need to be set in C++ via `GetWorldSettings()` rather than in a map
- No visual level design in the editor (all procedural) -- but this is already the plan for a Frogger game with procedural lanes
- The one minimal map file still needs to be created somehow (one-time operation)

**Sources:**
- [UE5 Enhanced Input in Pure C++ (Coding Babble)](https://codingbabble.com/posts/ue5-enhanced-input-in-pure-cpp/)
- [Game Mode and Game State (Official)](https://dev.epicgames.com/documentation/en-us/unreal-engine/game-mode-and-game-state-in-unreal-engine)
- [Player Start Actor (Official)](https://dev.epicgames.com/documentation/en-us/unreal-engine/player-start-actor-in-unreal-engine)
- [Enhanced Input (Official)](https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine)

---

## Comparison Matrix

| Approach | Maps | Input Assets | Headless | Reliability | Setup Cost |
|----------|------|-------------|----------|-------------|-----------|
| 1. Commandlet (C++) | Yes (fragile) | Yes | Yes | Low (crash-prone) | High |
| 2. Python Scripting | Yes | Likely | Yes | Medium | Low |
| 3. Remote Control API | No | No | No | N/A | N/A |
| 4. Editor Plugin/MCP | Yes | Yes | No | High (needs GUI) | High |
| 5. Computer Use | Yes | Yes | No | Low (brittle) | Medium |
| **6. C++ Auto-Setup** | **Partial** | **Yes** | **Yes** | **High** | **Low** |

---

## Recommendation: Hybrid Approach (6 + 2)

### Primary Strategy: Approach 6 -- C++ Auto-Setup

For this project (UnrealFrog), Approach 6 is ideal because:

1. **The game is already procedural.** Lane generation, hazard spawning, and scoring are all C++ systems. Having the GameMode also spawn the playing field (ground plane, lane visuals, UI) is a natural extension.

2. **Enhanced Input is fully solved.** The pure-C++ `NewObject<UInputAction>()` pattern eliminates all need for `.uasset` input files. The existing `EnhancedInput` module dependency in `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/UnrealFrog.Build.cs` is already in place.

3. **No binary asset dependencies.** Agents can write, compile, and test everything from the CLI. The entire game is defined in C++ source files that live in git.

4. **Your existing config already sets this up.** `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Config/DefaultEngine.ini` already has `GlobalDefaultGameMode=/Script/UnrealFrog.UnrealFrogGameMode` -- the C++ GameMode is already the global default.

### One-Time Bootstrap: Approach 2 -- Python Script for Minimal Map

For the single empty map file, run a one-time Python script:

```python
# create_minimal_map.py
import unreal

# Create empty level and save it
level_lib = unreal.EditorLevelLibrary
level_lib.new_level('/Game/Maps/FrogMap')
level_lib.save_current_level()
unreal.log("FrogMap created and saved.")
```

Run once:
```bash
"/Users/Shared/Epic Games/UE_5.7/Engine/Binaries/Mac/UnrealEditor-Cmd" \
  "/Users/randroid/Documents/Dev/Unreal/UnrealFrog/UnrealFrog.uproject" \
  -ExecutePythonScript="create_minimal_map.py"
```

Then commit the resulting `.umap` file. This is a one-time operation. After that, agents never need the editor again for map creation.

### Alternative: Skip the Map Entirely

If UE 5.7 creates a default empty map when the referenced map doesn't exist (which some versions do), you might not even need this step. The engine will create a transient world and the GameMode's `BeginPlay()` will handle everything. This needs testing but could eliminate the binary asset dependency entirely.

---

## Implementation Plan for UnrealFrog

1. **Create `UIA_Hop` / `UIMC_Frog` C++ classes** -- Pure C++ Enhanced Input with `NewObject` pattern
2. **Create `AFrogPlayerController`** -- Sets up input mapping context in `SetupInputComponent()`  
3. **Extend `AUnrealFrogGameMode`** -- Override `InitGame()` or `BeginPlay()` to spawn ground plane, lane visuals, PlayerStart equivalent
4. **Bootstrap minimal map** -- One-time Python script OR test if default map works
5. **Update `DefaultEngine.ini`** -- Point `GameDefaultMap` to the new map (or keep existing)
