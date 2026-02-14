# Conversation: 2026-02-13

| Field | Value |
|-------|-------|
| Date | 2026-02-13 |
| Session ID | `2676fd16-2f8c-492a-af3b-1239ff04855a` |
| Model | claude-opus-4-6 |
| Tokens | ~934,543 input, ~207 output |
| Duration | 3m 52s |

---

## User (06:09 UTC)

Research Unreal Engine 5 editor automation capabilities for the following questions. Search the web for up-to-date information (2025-2026):

1. PythonScriptPlugin + EditorScriptingUtilities: Can they create maps, place actors, set World Settings via UnrealEditor-Cmd? What Python APIs are available?

2. Remote Control API: What does it offer for controlling a running editor instance from external processes? Can it trigger PIE, set properties, call functions?

3. Automation Driver: What is UnrealAutomationDriver? Can it simulate input (keyboard/mouse) in PIE?

4. unreal-mcp (Model Context Protocol server for Unreal): Does it exist? What capabilities does it provide?

5. ExecCmds and console commands: What automation console commands are available for testing?

6. What's the best approach for a CI-friendly headless test runner that can also do gameplay validation (not just unit tests)?

Return a structured summary of findings for each question. Focus on practical, actionable information.

## Assistant (06:09 UTC)

🤖️

Let me research each of these topics thoroughly.

- **Used WebSearch**: `Unreal Engine 5 PythonScriptPlugin EditorScriptingUtilities automation create maps place actors Python API 2025 2026`

- **Used WebSearch**: `Unreal Engine 5 Remote Control API external process PIE trigger set properties call functions 2025`

- **Used WebSearch**: `Unreal Engine UnrealAutomationDriver simulate input keyboard mouse PIE testing 2025`

- **Used WebSearch**: `unreal-mcp Model Context Protocol server Unreal Engine 2025 2026`

- **Used WebSearch**: `Unreal Engine 5 ExecCmds console commands automation testing headless 2025`

- **Used WebSearch**: `Unreal Engine 5 CI headless test runner gameplay validation Gauntlet functional tests 2025 2026`

## Assistant (06:09 UTC)

Let me get more detailed information on several of these topics.

- **Used WebFetch** on `https://dev.epicgames.com/documentation/en-us/unreal-engine/scripting-the-unreal-editor-using-python`

- **Used WebFetch** on `https://dev.epicgames.com/documentation/en-us/unreal-engine/remote-control-api-http-reference-for-unreal-engine`

- **Used WebFetch** on `https://dev.epicgames.com/documentation/en-us/unreal-engine/automation-driver-in-unreal-engine`

- **Used WebFetch** on `https://github.com/chongdashu/unreal-mcp`

- **Used WebFetch** on `https://dev.epicgames.com/documentation/en-us/unreal-engine/run-automation-tests-in-unreal-engine`

- **Used WebFetch** on `https://dev.epicgames.com/documentation/en-us/unreal-engine/gauntlet-automation-framework-in-unreal-engine`

## Assistant (06:10 UTC)

Let me get the remaining details from the Gauntlet overview and the `flopperam/unreal-engine-mcp` project, plus some additional searches.

- **Used WebFetch** on `https://dev.epicgames.com/documentation/en-us/unreal-engine/gauntlet-automation-framework-overview-in-unreal-engine`

- **Used WebFetch** on `https://dev.epicgames.com/documentation/en-us/unreal-engine/running-gauntlet-tests-in-unreal-engine`

- **Used WebFetch** on `https://dev.epicgames.com/documentation/en-us/unreal-engine/functional-testing-in-unreal-engine`

- **Used WebFetch** on `https://github.com/flopperam/unreal-engine-mcp`

## Assistant (06:10 UTC)

Now let me do a couple more targeted searches to fill in gaps.

- **Used WebSearch**: `Unreal Engine 5 Python API create new map level EditorLevelLibrary new_level save_current_level world settings 2025`

- **Used WebSearch**: `Unreal Engine 5 Remote Control API start PIE play in editor console command trigger from HTTP 2025`

- **Used WebSearch**: `Unreal Engine EditorLevelSubsystem LevelEditorSubsystem Python spawn_actor_from_class set world settings commandlet headless`

- **Used WebSearch**: `Unreal Engine 5 "ke" console command "request play session" PIE automation exec command 2025`

- **Used WebSearch**: `Unreal Engine console command start PIE play session automation "GEditor" "RequestPlaySession" OR "AutomationOpenMap" OR "Autotest StartPIE"`

- **Used WebSearch**: `Unreal Engine 5 Python unreal.EditorActorSubsystem spawn_actor_from_class get_all_level_actors set_actor_location world_settings`

- **Used WebFetch** on `https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/EditorActorSubsystem?application_version=5.6`

- **Used WebFetch** on `https://forums.unrealengine.com/t/starting-pie-programmatically/1213447`

## Assistant (06:11 UTC)

Now I have comprehensive information on all six topics. Let me update the task and compile the findings.

- **Used TaskUpdate** on task 22

## Assistant (06:13 UTC)

Here is the structured research summary.

---

# UE5 Editor Automation Research Findings

## 1. PythonScriptPlugin + EditorScriptingUtilities

### Can it create maps, place actors, set World Settings via UnrealEditor-Cmd?

**Yes, with caveats.** The Python scripting system is quite capable for editor-time automation.

### Headless Commandlet Execution

The `-run=pythonscript` commandlet runs Python scripts without opening the Editor UI:

```bash
UnrealEditor-Cmd "Project.uproject" -run=pythonscript -script="path/to/script.py"
```

This is described as "very fast" and works in headless mode. **Important:** This commandlet does not automatically load levels, so your script must call `load_level()` first.

### Key Python APIs

**Level/Map Management** (`unreal.EditorLevelLibrary`, being superseded by `unreal.LevelEditorSubsystem`):
- `new_level(level_name)` -- creates a new map
- `load_level(level_name)` -- loads an existing map
- `save_current_level()` -- saves the current map
- Access via: `unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)`

**Actor Placement** (`unreal.EditorActorSubsystem`):
- `spawn_actor_from_class(actor_class, location, rotation, transient)` -- spawn by class
- `spawn_actor_from_object(object_to_use, location, rotation, transient)` -- spawn from Factory, Archetype, Blueprint, Class, or Asset
- `get_all_level_actors()` -- retrieve all actors in the level
- `destroy_actor(actor)` / `destroy_actors(actors)` -- remove actors
- `duplicate_actor(actor, to_world, offset)` -- duplicate
- `set_actor_transform(actor, world_transform)` -- reposition
- `get_actor_reference(path_to_actor)` -- find actor by path
- Full selection management: `set_selected_level_actors()`, `select_all()`, etc.

**Property Access** (on any UObject):
- `obj.set_editor_property("PropertyName", value)` -- modify with Editor callbacks
- `obj.get_editor_property("PropertyName")` -- read properties
- This works on World Settings actors too

**Asset Operations** (`unreal.EditorAssetLibrary`, `unreal.AssetTools`):
- Asset search, rename, duplicate, delete

**Transactions:** `unreal.ScopedEditorTransaction` bundles operations into undo/redo units.

### World Settings

Not a dedicated API, but accessible by getting the World Settings actor from the level and using `set_editor_property()` on it. For example:
```python
world = unreal.EditorLevelLibrary.get_editor_world()
world_settings = world.get_world_settings()
world_settings.set_editor_property("property_name", value)
```

### Limitations
- The Python API is marked "Experimental" even in UE 5.6/5.7
- `EditorLevelLibrary` functions are being deprecated in favor of `EditorActorSubsystem` and `LevelEditorSubsystem`
- Commandlet mode has no viewport, so visual operations fail
- Cannot start PIE from Python directly (no exposed binding)

---

## 2. Remote Control API

### What does it offer?

A **REST-like HTTP + WebSocket API** that runs as a web server inside the editor, listening on port 30010 by default. External processes on the same network can control a running editor instance.

### HTTP Endpoints

| Endpoint | Method | Purpose |
|----------|--------|---------|
| `/remote/info` | GET | List all available routes |
| `/remote/object/call` | PUT | Call any `BlueprintCallable` function on any UObject |
| `/remote/object/property` | PUT | Read/write any public, non-const property |
| `/remote/object/describe` | PUT | Get metadata about a UObject (all properties, types) |
| `/remote/object/thumbnail` | PUT | Get asset thumbnail image |
| `/remote/search/assets` | PUT | Query the Asset Registry |
| `/remote/batch` | PUT | Batch multiple requests sequentially |
| `/remote/object/event` | PUT | (Experimental) Monitor property changes |

### Can it trigger PIE?

**Not directly via a dedicated endpoint.** However, you can call any `BlueprintCallable` function via `/remote/object/call`, which theoretically lets you invoke editor utility functions. The standard approach for triggering PIE from code is:

```cpp
FPlayWorldCommands::GlobalPlayWorldActions->ExecuteAction(
    FPlayWorldCommands::Get().PlayInViewport.ToSharedRef()
);
```

This C++ code (requiring `#include "Kismet2/DebuggerCommands.h"` and the `UnrealEd` module) is not directly exposed as a BlueprintCallable function, so **triggering PIE via Remote Control requires wrapping it in a custom BlueprintCallable function** or using an Editor Utility Widget/Blueprint.

### PIE Mode Access

When PIE is running, object paths gain a `UEDPIE_X_` prefix (X = PIE instance ID). Properties must be `BlueprintVisible` and not `BlueprintReadOnly` to be modified during PIE. You can read and modify game state of a running PIE session through the standard property/function endpoints.

### Configuration
- Enable the **Remote Control API** plugin (Messaging category)
- Start with console command: `WebControl.StartServer`
- Port configurable in Project Settings > Web Remote Control
- Experimental routes require `WebControl.EnableExperimentalRoutes = 1` in DefaultEngine.ini

---

## 3. Automation Driver

### What is it?

The `IAutomationDriverModule` is a **built-in UE module for simulating user input** with fluent syntax, primarily targeting Slate UI testing.

### Supported Input Types
- Mouse: movement, clicks, double-clicks, scrolling, drag-and-drop
- Keyboard: typing, key press/release, chords (e.g., Ctrl+Shift+S)
- Focus management
- **Not supported (yet):** Touch gestures, gamepad/controller input, motion

### Key APIs

```cpp
// Enable/disable
IAutomationDriverModule::Get().Enable();   // blocks all real platform input
IAutomationDriverModule::Get().Disable();

// Create a driver instance
FAutomationDriverPtr Driver = IAutomationDriverModule::Get().CreateDriver();

// Find elements by metadata ID (recommended)
FDriverElementRef Element = Driver->FindElement(By::Id("MyButtonId"));

// Find by path (tag, type, hierarchy)
FDriverElementRef Element = Driver->FindElement(By::Path("#Suite//Form//<SEditableText>"));

// Find by cursor position
FDriverElementRef Element = Driver->FindElement(By::Cursor());

// Perform actions
Element->Click();
Element->Type(TEXT("Hello World"));

// Sequence-based actions
FDriverSequenceRef Seq = Driver->CreateSequence();
Seq->Actions()
   .MoveToElement(By::Id("Submit"))
   .Click(EMouseButtons::Left);
Seq->Perform();

// Waiting
Driver->Wait(Until::ElementIsVisible(Element, FWaitTimeout::InSeconds(3)));
```

### Sync vs Async
- **Synchronous API:** Simpler but CANNOT run on GameThread (would deadlock). Use from a thread pool.
- **Asynchronous API:** Can run on GameThread but requires more complex coordination.

### Can it simulate gameplay input in PIE?

**No, not directly.** The Automation Driver is specifically designed for **Slate UI element interaction**, not for gameplay input (e.g., WASD movement, mouse look in a 3D viewport). It operates on Slate widgets, not on the game world's input system. For gameplay input simulation, you would need the Enhanced Input system's injection APIs or `APlayerController::InputKey()`/`InputAxis()`.

### Key Limitation
- Only operates on Slate-based UI elements
- Requires `FDriverMetaData::Id()` metadata tags on widgets for reliable element finding
- Source code at: `Engine/Source/Developer/AutomationDriver/`

---

## 4. Unreal MCP (Model Context Protocol Servers)

### Does it exist?

**Yes, multiple implementations exist**, all community-driven (not official Epic):

#### A. [chongdashu/unreal-mcp](https://github.com/chongdashu/unreal-mcp) (1.4k stars)
- **Architecture:** C++ plugin (TCP server on port 55557) + Python MCP server bridge
- **Status:** Experimental, breaking changes expected
- **Capabilities:**
  - Actor management: create, delete, transform, query, find by name
  - Blueprint development: create classes, add components, compile, spawn
  - Blueprint node graph: add event nodes, connect nodes, create variables, function calls
  - Editor viewport control: focus, camera orientation
- **Requires:** UE 5.5+, Python 3.12+, Visual Studio for C++ compilation
- **Compatible with:** Claude Desktop, Cursor, Windsurf

#### B. [flopperam/unreal-engine-mcp](https://github.com/flopperam/unreal-engine-mcp)
- **Architecture:** Similar C++ plugin + Python server stack
- **Differentiators:** Extensive world-building tools (towns, castles, mazes, mansions), deep Blueprint graph analysis (23+ node types), embedded browser UI panel, text/image-to-3D generation
- **Capabilities:** All of the above plus `create_town()`, `create_castle_fortress()`, `create_maze()`, physics/material management, autonomous multi-step execution
- **Requires:** UE 5.5-5.7
- **License:** MIT

#### C. [Docker-based MCP server](https://hub.docker.com/r/mcp/unreal-engine-mcp-server)
- Controls Unreal Engine via Remote Control API (no custom plugin required)
- TypeScript-based, containerized

### Relevance to PlayUnreal

The MCP approach is interesting but **not ideal for CI/headless testing**. These tools are designed for interactive AI-assisted development, not automated test execution. However, the `chongdashu` plugin's TCP socket protocol could be adapted for a PlayUnreal-style test harness.

---

## 5. ExecCmds and Console Commands

### Automation Console Commands

```bash
# Run specific tests
-ExecCmds="Automation RunTest Test1+Test2;Quit"

# Run all tests under a hierarchy
-ExecCmds="Automation RunTest MySet.MySubSet;Quit"

# Run tests in a group
-ExecCmds="Automation RunTest Group:MyGroup;Quit"

# List all available tests
-ExecCmds="Automation List"

# Run all tests
-ExecCmds="Automation RunAll"

# Run by filter
-ExecCmds="Automation RunFilter <filter_name>"
```

### Headless/CI Flags

```bash
UnrealEditor-Cmd "Project.uproject" \
  -ExecCmds="Automation RunTests <TestName>;Quit" \
  -stdout           # Output to stdout
  -unattended       # No user prompts
  -NOSPLASH         # Skip splash screen
  -NullRHI          # No rendering (headless)
  -ReportExportPath="path/to/results"  # JSON + HTML test report
  -ResumeRunTest    # Resume from last failure (reads existing report JSON)
```

### Other Useful Console Commands

- `WebControl.StartServer` -- Start Remote Control API server
- `Automation Quit` -- Exit after tests
- `-ResumeOnCriticalFailure` -- Auto-resume up to 3 times after crashes (Gauntlet)

---

## 6. Best Approach for CI-Friendly Headless Test Runner with Gameplay Validation

### Recommended Architecture: Layered Testing Pyramid

#### Layer 1: Unit Tests (Headless, Fast)
- **Tool:** Built-in Automation Test Framework with `-NullRHI`
- **What:** Pure logic tests (scoring, state machine, lane config)
- **How:** `UnrealEditor-Cmd -ExecCmds="Automation RunTests UnrealFrog;Quit" -NullRHI -unattended -stdout`
- **CI suitability:** Excellent -- no GPU required, fast, deterministic

#### Layer 2: Functional Tests (Needs Rendering, Medium Speed)
- **Tool:** `AFunctionalTest` actors placed in test maps + Gauntlet
- **What:** Gameplay validation -- spawn frog, simulate hop, verify collision response, check score
- **How:** Place `AFunctionalTest` subclass actors in a test map. Override `PrepareTest()`, `IsReady()`, and handle `OnTestStart` delegate. Call `FinishTest()` when assertions complete.
- **Run via Gauntlet:**
  ```bash
  RunUAT RunUnreal -test=UE.EditorAutomation \
    -runtest="UnrealFrog.Functional" \
    -project="path/to/UnrealFrog.uproject" \
    -build=editor
  ```
- **CI suitability:** Good -- needs GPU but validates actual gameplay

#### Layer 3: Integration/Smoke Tests (Full Editor, Slow)
- **Tool:** Custom C++ test controller + Remote Control API
- **What:** End-to-end validation -- launch editor, open map, start PIE, inject input, capture state
- **Architecture for PlayUnreal:**
  1. Launch `UnrealEditor` (not Cmd) with Remote Control API enabled
  2. External Python/shell process connects to `http://localhost:30010`
  3. Use `/remote/object/call` to invoke game functions (hop, pause)
  4. Use `/remote/object/property` to read game state (frog position, score, lives, game state enum)
  5. Wrap a custom `BlueprintCallable` function around `FPlayWorldCommands` to trigger PIE via HTTP
  6. Take screenshots via `HighResScreenshot` console command

### Gauntlet Framework (CI Orchestration)

Gauntlet is the **official CI orchestration layer**. It does not define how tests work internally -- it manages the lifecycle:

- **Architecture:** C# scripts that run via UAT (Unreal Automation Tool)
- **Tiers:** Core interfaces (`ITestNode`, `TestExecutor`) > Platform implementations (`TargetDeviceMac`, etc.) > Unreal-specific (`UnrealTestNode`, `UnrealSession`)
- **Built-in test types:**
  - `UE.BootTest` / `UE.EditorBootTest` -- just launch and verify no crash
  - `UE.EditorAutomation` / `UE.TargetAutomation` -- run Automation Framework tests
  - `UE.Networking` -- multiplayer tests
- **Key commands:**
  ```bash
  # Editor automation (runs tests in editor)
  RunUAT RunUnreal -test=UE.EditorAutomation -runtest=TestName -project=path -build=editor

  # Packaged game automation
  RunUAT RunUnreal -test=UE.TargetAutomation -runtest=TestName -project=path -build=path/to/build

  # Boot test (smoke test -- just verify launch)
  RunUAT RunUnreal -test=UE.BootTest -project=path -build=path/to/build
  ```
- **Crash recovery:** `-ResumeOnCriticalFailure` retries up to 3 times

### Recommended Approach for UnrealFrog PlayUnreal

Given the project constraints (macOS, UE 5.7, agent-driven development), the most practical architecture is:

1. **Unit/logic tests:** Continue using the existing Automation Framework tests with `-NullRHI` headless mode (already working per Task 11)
2. **Gameplay validation:** Write `AFunctionalTest` subclasses in C++ that:
   - Spawn the frog, obstacles, and logs programmatically
   - Use `APlayerController::InputKey()` to simulate input
   - Assert game state via direct C++ access (no network overhead)
   - Call `FinishTest()` with pass/fail
3. **CI runner:** Use `UnrealEditor-Cmd` with `-ExecCmds` to run functional tests headless (some tests may need `-NullRHI` omitted for physics)
4. **Interactive QA (PlayUnreal):** Build a thin Remote Control API wrapper script that:
   - Launches the editor with Remote Control enabled
   - Connects via HTTP on port 30010
   - Triggers PIE via a custom `BlueprintCallable` wrapper
   - Reads game state via property access
   - Injects input via function calls on the PlayerController

The **functional test approach** (Layer 2) gives the best balance of CI-friendliness and gameplay validation. It runs inside the engine's own test framework, has access to all game systems, and can be executed headless via Gauntlet or ExecCmds.

---

### Sources

- [Scripting the Unreal Editor Using Python (UE 5.7)](https://dev.epicgames.com/documentation/en-us/unreal-engine/scripting-the-unreal-editor-using-python)
- [Python Editor Script Plugin (UE 5.7)](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/PluginIndex/PythonScriptPlugin)
- [EditorActorSubsystem Python API (UE 5.6)](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/EditorActorSubsystem?application_version=5.6)
- [Remote Control API HTTP Reference (UE 5.7)](https://dev.epicgames.com/documentation/en-us/unreal-engine/remote-control-api-http-reference-for-unreal-engine)
- [Remote Control for Unreal Engine (UE 5.7)](https://dev.epicgames.com/documentation/en-us/unreal-engine/remote-control-for-unreal-engine)
- [Remote Control Quick Start (UE 5.7)](https://dev.epicgames.com/documentation/en-us/unreal-engine/remote-control-quick-start-for-unreal-engine)
- [Automation Driver in Unreal Engine (UE 5.7)](https://dev.epicgames.com/documentation/en-us/unreal-engine/automation-driver-in-unreal-engine)
- [chongdashu/unreal-mcp (GitHub)](https://github.com/chongdashu/unreal-mcp)
- [flopperam/unreal-engine-mcp (GitHub)](https://github.com/flopperam/unreal-engine-mcp)
- [Docker MCP Server](https://hub.docker.com/r/mcp/unreal-engine-mcp-server)
- [Run Automation Tests (UE 5.7)](https://dev.epicgames.com/documentation/en-us/unreal-engine/run-automation-tests-in-unreal-engine)
- [Automation System User Guide (UE 5.7)](https://dev.epicgames.com/documentation/en-us/unreal-engine/automation-system-user-guide-in-unreal-engine)
- [Gauntlet Automation Framework (UE 5.7)](https://dev.epicgames.com/documentation/en-us/unreal-engine/gauntlet-automation-framework-in-unreal-engine)
- [Gauntlet Framework Overview (UE 5.7)](https://dev.epicgames.com/documentation/en-us/unreal-engine/gauntlet-automation-framework-overview-in-unreal-engine)
- [Running Gauntlet Tests (UE 5.7)](https://dev.epicgames.com/documentation/en-us/unreal-engine/running-gauntlet-tests-in-unreal-engine)
- [Functional Testing (UE 5.7)](https://dev.epicgames.com/documentation/en-us/unreal-engine/functional-testing-in-unreal-engine)
- [Automation Test Framework (UE 5.7)](https://dev.epicgames.com/documentation/en-us/unreal-engine/automation-test-framework-in-unreal-engine)
- [Starting PIE Programmatically (Epic Forums)](https://forums.unrealengine.com/t/starting-pie-programmatically/1213447)
- [UnrealHelpers Python Utilities (GitHub)](https://github.com/Short-Fuse-Games/UnrealHelpers)
