# Spike: Editor Automation for UnrealFrog

*Sprint 2 Research | DevOps Engineer*

## Summary

This spike investigates how to programmatically control the Unreal Editor from CLI/agents for map creation, gameplay validation, and CI testing. The goal: enable richer editor interaction in Sprint 3+ beyond headless unit tests.

## Findings

### 1. PythonScriptPlugin + EditorScriptingUtilities

**Verdict: Good for asset/map creation. Cannot start PIE.**

The Python scripting system runs via `UnrealEditor-Cmd -run=pythonscript -script="path.py"` or `-ExecutePythonScript="path.py"`. It provides full access to:

- **Map management**: `unreal.EditorLevelLibrary.new_level()`, `load_level()`, `save_current_level()`
- **Actor spawning**: `unreal.EditorActorSubsystem.spawn_actor_from_class()`
- **Property access**: `obj.set_editor_property()` / `get_editor_property()` on any UObject
- **World Settings**: Get via `world.get_world_settings()`, modify properties
- **Asset operations**: Search, rename, duplicate, delete assets

**Limitations:**
- Marked "Experimental" even in UE 5.7
- `EditorLevelLibrary` being deprecated → use `EditorActorSubsystem` and `LevelEditorSubsystem`
- Commandlet mode has no viewport — visual operations fail
- **Cannot start PIE from Python** (no exposed binding)

**Our current usage:** `Tools/CreateMap/create_frog_map.py` creates the empty FroggerMain map. This pattern works well for headless asset setup.

### 2. Remote Control API

**Verdict: Best option for controlling a running editor/game from external processes.**

A REST HTTP + WebSocket API running on port 30010 inside the editor.

| Endpoint | Purpose |
|----------|---------|
| `PUT /remote/object/call` | Call any `BlueprintCallable` function |
| `PUT /remote/object/property` | Read/write any public property |
| `PUT /remote/object/describe` | Get UObject metadata |
| `PUT /remote/search/assets` | Query Asset Registry |
| `PUT /remote/batch` | Batch multiple requests |

**Key capabilities for PlayUnreal:**
- Read game state: frog position, score, lives, game state enum
- Call game functions: hop, pause, start
- Modify properties: set lives, set position for test scenarios

**PIE limitation:** No direct "start PIE" endpoint. Requires wrapping `FPlayWorldCommands` in a custom `BlueprintCallable` C++ function, then calling it via HTTP. During PIE, object paths gain `UEDPIE_X_` prefix.

**Setup:** Enable `Remote Control API` plugin, start via `WebControl.StartServer` console command.

### 3. Automation Driver

**Verdict: UI testing only. Not suitable for gameplay input.**

`IAutomationDriverModule` provides fluent Slate UI testing:
```cpp
FDriverElementRef Element = Driver->FindElement(By::Id("MyButton"));
Element->Click();
Element->Type(TEXT("Hello"));
```

**Not suitable for gameplay** — it operates on Slate widgets, not the game world's input system. For gameplay input simulation, use `APlayerController::InputKey()` or Enhanced Input injection APIs instead.

### 4. Unreal MCP Servers

**Verdict: Interesting for AI-assisted development, not for CI testing.**

Two community implementations exist:
- **chongdashu/unreal-mcp** (1.4k stars): C++ plugin + Python MCP bridge. Actor/Blueprint management.
- **flopperam/unreal-engine-mcp**: Extended world-building tools, Blueprint graph analysis.

Both require the full editor running and are designed for interactive AI-assisted development, not automated testing. The TCP socket protocol from chongdashu could theoretically be adapted for a test harness.

### 5. Console Commands for Testing

```bash
# Run specific test group headless
UnrealEditor-Cmd "Project.uproject" \
  -ExecCmds="Automation RunTests UnrealFrog;Quit" \
  -NullRHI -unattended -NoSound -NoPause \
  -ReportExportPath="TestResults/"

# List available tests
-ExecCmds="Automation List UnrealFrog;Quit"

# Resume from last failure
-ResumeRunTest

# Crash recovery (Gauntlet)
-ResumeOnCriticalFailure
```

**ReportExportPath** generates JSON + HTML test reports — useful for CI dashboards.

### 6. Gauntlet Framework

The official CI orchestration layer. C# scripts run via UAT:

```bash
# Editor automation (tests in editor)
RunUAT RunUnreal -test=UE.EditorAutomation -runtest="UnrealFrog" -project=path

# Boot test (verify launch without crash)
RunUAT RunUnreal -test=UE.BootTest -project=path -build=path/to/build

# Crash recovery: retries up to 3 times
-ResumeOnCriticalFailure
```

## Recommended Architecture for Sprint 3+

### Testing Pyramid

```
                    ┌─────────────┐
                    │  Layer 3    │  Interactive QA (Remote Control API)
                    │  PlayUnreal │  Full editor + PIE + external driver
                    ├─────────────┤
                    │  Layer 2    │  Functional Tests (AFunctionalTest)
                    │  Gameplay   │  Spawned in test maps, input simulation
                    ├─────────────┤
                    │  Layer 1    │  Unit Tests (current)
                    │  Logic      │  Headless, -NullRHI, fast
                    └─────────────┘
```

### Layer 1: Unit Tests (Already Done)

Our current `Source/UnrealFrog/Tests/*.cpp` tests run headless via:
```bash
Tools/PlayUnreal/run-tests.sh
```
Fast, deterministic, no GPU needed. Covers: scoring, state machine, movement math, collision logic.

### Layer 2: Functional Tests (Sprint 3 Priority)

Write `AFunctionalTest` subclasses that run inside the engine:
- Spawn actors programmatically
- Simulate input via `APlayerController::InputKey()`
- Assert game state via direct C++ access
- Call `FinishTest(EFunctionalTestResult::Succeeded)` when done

**Advantages:** No network overhead, full engine access, can run headless via Gauntlet.

**Implementation plan:**
1. Create `Source/UnrealFrog/Tests/Functional/` directory
2. Write `AFroggerFunctionalTest` base class with common setup
3. Individual test actors: `AFT_HopAndScore`, `AFT_CarCollision`, `AFT_RiverLanding`
4. Create test map: `Content/Maps/FroggerTest.umap` with test actors placed
5. Run via: `UnrealEditor-Cmd -ExecCmds="Automation RunTests UnrealFrog.Functional;Quit"`

### Layer 3: PlayUnreal Interactive Driver (Sprint 3 Stretch)

A Python/shell script that controls a running editor via Remote Control API:

```
┌──────────────┐     HTTP :30010     ┌──────────────────┐
│  play_test.py │ ──────────────────► │  UnrealEditor    │
│  (external)   │ ◄────────────────── │  + PIE running   │
│               │    JSON responses   │  + Remote Control│
└──────────────┘                     └──────────────────┘
```

**Steps:**
1. Launch `UnrealEditor` with Remote Control enabled
2. Custom C++ `UFUNCTION(BlueprintCallable)` wraps PIE start
3. Python script connects, triggers PIE, waits for Playing state
4. Script reads game state, injects inputs, captures screenshots
5. Script asserts acceptance criteria, reports results

**Prerequisite:** Custom C++ "PIE bridge" function:
```cpp
UFUNCTION(BlueprintCallable, Category = "Testing")
static void StartPIEForTesting()
{
    // Wrap FPlayWorldCommands to trigger PIE
}
```

## Recommendation

**Sprint 3 focus: Layer 2 (Functional Tests).**

Functional tests give the best ROI — they validate actual gameplay in-engine without requiring an external driver. They integrate with the existing Automation Framework and can run headless in CI.

Layer 3 (PlayUnreal interactive driver) is a stretch goal for Sprint 3 or Sprint 4. It requires more infrastructure (Remote Control plugin, PIE bridge, Python driver) but enables the richest QA validation.

## Action Items for Sprint 3

1. Enable `Remote Control API` plugin in `.uproject`
2. Write `AFroggerFunctionalTest` base class
3. Create 3-5 functional test actors covering core gameplay
4. Add `Gauntlet` integration to `run-tests.sh`
5. (Stretch) Implement PIE bridge + Python Remote Control driver
