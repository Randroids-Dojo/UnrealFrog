# Conversation: 2026-02-15

| Field | Value |
|-------|-------|
| Date | 2026-02-15 |
| Session ID | `9cad10df-a2ee-4650-adec-4a8034993a77` |
| Model | claude-sonnet-4-5-20250929 |
| Tokens | ~113,046 input, ~5 output |
| Duration | 36s |

---

## User (17:36 UTC)

You are the Engine Architect providing retrospective feedback on the Post-Sprint 7 Stakeholder Review for the UnrealFrog project (UE 5.7 C++ 3D Frogger).

## Context
The stakeholder played the game after Sprint 7 and found 3 critical issues that 162 automated tests missed:

1. **"No visible differences when playing compared to previous sprints."** — Difficulty scaling works at the code level (hazards speed up 15% per wave) but the player can't perceive it.

2. **"Long-standing bug: explosion barely visible at bottom-left, +score text barely visible at top-left."** — Score pops hardcoded to screen position `FVector2D(20 + textLen*10, 10)` in FroggerHUD.cpp:116 instead of frog's projected position. Death VFX (scale 0.5→3.0) too small from Z=2200 camera. Home slot celebrations hardcoded to `Col * 100, 14 * 100, 50`.

3. **"PlayUnreal is not a real tool."** — PlayUnrealTest.cpp calls methods directly, never sends real inputs. Need HTTP server + Python client for scripted gameplay.

## Your Task
Read the relevant source files to understand the technical details, then provide your Engine Architect perspective. Focus on:

- **Technical root cause**: Why are VFX too small? What's the correct approach for camera-distance-relative scaling?
- **Score pop fix**: How should world-to-screen projection work in UE5 Canvas HUD? (Canvas->Project? GetPlayerController()->ProjectWorldLocationToScreen?)
- **PlayUnreal architecture**: HTTP server vs Remote Control API vs PythonScriptPlugin — which is most practical for UE 5.7?
- **Estimated complexity**: How many lines of code for each fix?

Read the following files to inform your analysis:
- Source/UnrealFrog/Private/Core/FroggerHUD.cpp
- Source/UnrealFrog/Private/Core/FroggerVFXManager.cpp
- Source/UnrealFrog/Private/Core/UnrealFrogGameMode.cpp (just the Tick and VFX sections)

Keep your response to 20-30 lines. Be technical and specific. Output your response as plain text (it will be read by the team lead).

## User (17:36 UTC)

<command-message>unreal-conventions</command-message>
<command-name>unreal-conventions</command-name>
<skill-format>true</skill-format>
Base directory for this skill: /Users/randroid/Documents/Dev/Unreal/UnrealFrog/.claude/skills/unreal-conventions

## Naming Conventions

| Type | Prefix | Example |
|------|--------|---------|
| Actor | A | AFrogCharacter |
| UObject | U | UScoreSubsystem |
| Struct | F | FLaneConfig |
| Enum | E | EGameState |
| Interface | I | IHazardInterface |
| Delegate | F...Delegate | FOnScoreChanged |
| Template | T | TArray, TMap |

## File Organization

- Header (.h) and implementation (.cpp) always paired
- One primary class per file pair
- File name matches class name without prefix: `FrogCharacter.h` for `AFrogCharacter`
- Module folders: `/Public/` for headers, `/Private/` for implementation

## UPROPERTY Specifiers

```cpp
// Editable in editor, readable in Blueprint
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
float HopDistance = 100.0f;

// Visible but not editable, useful for debugging
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
int32 CurrentLives = 3;

// Internal only, not exposed
UPROPERTY()
FTimerHandle RespawnTimerHandle;
```

## UFUNCTION Specifiers

```cpp
// Blueprint callable
UFUNCTION(BlueprintCallable, Category = "Movement")
void Hop(FVector Direction);

// Event that Blueprint can implement
UFUNCTION(BlueprintImplementableEvent, Category = "Events")
void OnFrogDied();

// C++ implementation with Blueprint override
UFUNCTION(BlueprintNativeEvent, Category = "Scoring")
int32 CalculateScore();
```

## Delegate Patterns

```cpp
// Dynamic multicast (Blueprint compatible)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, int32, NewScore);

// Always verify target has UFUNCTION before AddDynamic
ScoreDelegate.AddDynamic(this, &AMyActor::HandleScoreChanged);
```

## Object Lifecycle

- Use `CreateDefaultSubobject<T>()` in constructors for components
- Use `NewObject<T>()` for UObject creation at runtime
- Never use raw `new` for UObjects — they are garbage collected
- Use `TWeakObjectPtr<T>` for non-owning references
- Use `UPROPERTY()` to prevent GC from collecting referenced objects

## Common Patterns

- **BeginPlay()** for runtime initialization (not constructor)
- **Tick()** only when necessary — prefer timers and events
- **GetWorld()->GetSubsystem<T>()** for accessing game subsystems
- **Cast<T>()** for safe type casting (returns nullptr on failure)

## User (17:36 UTC)

<command-message>unreal-build</command-message>
<command-name>unreal-build</command-name>
<skill-format>true</skill-format>
Base directory for this skill: /Users/randroid/Documents/Dev/Unreal/UnrealFrog/.claude/skills/unreal-build

## Build System

### Module Setup (Build.cs)

```csharp
// Source/UnrealFrog/UnrealFrog.Build.cs
using UnrealBuildTool;

public class UnrealFrog : ModuleRules
{
    public UnrealFrog(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine", "InputCore",
            "EnhancedInput", "UMG"
        });
    }
}
```

### Target Files

- `UnrealFrog.Target.cs` — Game target (shipping build)
- `UnrealFrogEditor.Target.cs` — Editor target (development)

## Testing Framework

### Automation Tests

```cpp
#include "Misc/AutomationTest.h"

// Simple unit test
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FScoreSystemTest,
    "UnrealFrog.Gameplay.ScoreSystem",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FScoreSystemTest::RunTest(const FString& Parameters)
{
    // Arrange
    UScoreSubsystem* Score = NewObject<UScoreSubsystem>();

    // Act
    Score->AddPoints(100);

    // Assert
    TestEqual("Score should be 100", Score->GetCurrentScore(), 100);
    return true;
}
```

### Functional Tests (In-Game)

```cpp
// AFroggerFunctionalTest inherits from AFunctionalTest
// Place in test maps, run via automation framework
UCLASS()
class AFrogHopTest : public AFunctionalTest
{
    GENERATED_BODY()
public:
    virtual void StartTest() override;
    virtual void Tick(float DeltaTime) override;
};
```

### Running Tests

```bash
# Run all project tests
UnrealEditor-Cmd UnrealFrog.uproject -ExecCmds="Automation RunTests UnrealFrog" -NullRHI -NoSound

# Run specific test group
UnrealEditor-Cmd UnrealFrog.uproject -ExecCmds="Automation RunTests UnrealFrog.Gameplay"

# Run with Gauntlet (CI-friendly)
RunUAT.bat RunUnrealTests -project=UnrealFrog.uproject -test=UnrealFrog
```

## Asset Validation

### Data Validation Plugin

Enable `DataValidation` plugin. Create validators:

```cpp
UCLASS()
class UAssetNamingValidator : public UEditorValidatorBase
{
    GENERATED_BODY()
public:
    virtual EDataValidationResult IsValid(TArray<FText>& ValidationErrors) override;
};
```

### Naming Convention Checks

| Asset Type | Prefix | Example |
|-----------|--------|---------|
| Static Mesh | SM_ | SM_Frog |
| Skeletal Mesh | SK_ | SK_FrogAnimated |
| Material | M_ | M_Road |
| Material Instance | MI_ | MI_Road_Wet |
| Texture | T_ | T_Frog_Albedo |
| Sound Wave | SW_ | SW_Hop |
| Sound Cue | SC_ | SC_HopWithVariation |
| Blueprint | BP_ | BP_FrogCharacter |
| Widget Blueprint | WBP_ | WBP_HUD |
| Level | LVL_ | LVL_MainGame |

## Python Editor Scripting

```python
import unreal

# Import an asset
task = unreal.AssetImportTask()
task.filename = "path/to/model.fbx"
task.destination_path = "/Game/Meshes/Characters"
task.automated = True
task.replace_existing = True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

# Validate assets
validator = unreal.EditorValidatorSubsystem()
validator.validate_assets_with_settings(assets, settings)
```

## Assistant (17:36 UTC)

🤖️

Let me read the relevant source files to understand the technical details.

- **Used Read** on `Source/UnrealFrog/Private/Core/FroggerHUD.cpp`

- **Used Read** on `Source/UnrealFrog/Private/Core/FroggerVFXManager.cpp`

- **Used Read** on `Source/UnrealFrog/Public/Core/FroggerVFXManager.h`

## Assistant (17:37 UTC)

## Engine Architect Perspective: Sprint 7 Retrospective

**1. VFX Too Small (Death Explosion, Home Celebrations)**

Root cause: Hardcoded world-space scaling (0.5→3.0 units) viewed from camera Z=2200. Basic trigonometry: a 3.0-unit sphere projects to ~1.57 pixels at that distance with FOV 50.

Correct approach: **Camera-distance-relative scaling.** Query player camera distance, scale VFX by `Distance/ReferenceDistance`. Reference distance 500 → 3.0 units would be ~13.2 units at camera Z=2200.

Fix location: `FroggerVFXManager.cpp:58` — replace `EndScale = 3.0f` with `EndScale = CalculateScreenSpaceScale(3.0f, Location)`. Add method: `float CalculateScreenSpaceScale(float DesiredScreenSize, FVector WorldPos)` — gets player camera, computes distance, returns `DesiredScreenSize * (Distance / 500.0)`.

Estimated: ~30 lines (helper method + 3 callsites).

**2. Score Pop Hardcoded Position (Top-Left, Not Frog)**

Root cause: `FroggerHUD.cpp:116` — `Pop.Position = FVector2D(20 + textLen*10, 10)`. Needs world-to-screen projection of frog actor location.

Correct approach: `APlayerController::ProjectWorldLocationToScreen(FVector WorldLocation, FVector2D& ScreenLocation, bool bPlayerViewportRelative)`. Call in `DrawScorePops` with frog's `GetActorLocation()`.

Fix location: `FroggerHUD.cpp:113-116` — query GameMode for frog actor ref, project location, use result as Pop.Position.

Caveat: Canvas HUD uses absolute screen coordinates. `bPlayerViewportRelative=false`. Must account for viewport offset in split-screen (not applicable here).

Estimated: ~15 lines (frog ref query + projection call + fallback to screen center if projection fails).

**3. PlayUnreal Not Real (Direct Method Calls, No Inputs)**

Current state: `PlayUnrealTest.cpp` instantiates actors, calls methods. Zero input simulation. HTTP server is overkill — we don't need external clients.

Correct approach: **Automation Driver Plugin** (ships with UE 5.7). Programmatic input injection via `FAutomationDriverModule::Get().GetDriver()->PressKey(EKeys::W)`. Supports mouse, keyboard, gamepad. Same API used by Gauntlet for Fortnite QA.

Alternative (lower complexity): **Enhanced Input action triggers via C++.** Query input subsystem, manually trigger action via `UEnhancedInputLocalPlayerSubsystem::InjectInputForAction()`. Requires valid player controller.

Recommendation: Automation Driver for E2E, Enhanced Input injection for unit tests. Remote Control API is Blueprint-focused, not ideal for scripted C++ scenarios.

Estimated: ~100 lines (driver setup, key press sequence, state polling, screenshot capture via `FScreenshotRequest::RequestScreenshot`).

**4. Difficulty Scaling Invisible**

Tests verify hazard speed increases 15%/wave. Players can't perceive it. This is a **design problem, not a systems problem.** Game Designer owns feel. Engine Architect can provide telemetry (HUD debug overlay showing actual hazard speeds), but tuning the curve is not my domain.

Technical enabler: Add `DrawDebugString` in `HUD::DrawHUD` with current wave speed multiplier when `-vfxdebug` flag is set. ~5 lines.

---

**Total estimated complexity: ~150 lines across 3 files (VFXManager, HUD, PlayUnrealTest).** All fixes are isolated — no seam test explosions. Priority: Score pop projection (P0, breaks game feel), VFX scaling (P1, polish), PlayUnreal (P2, infrastructure debt).
