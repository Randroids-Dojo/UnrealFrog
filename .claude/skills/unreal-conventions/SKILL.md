---
name: unreal-conventions
description: Unreal Engine 5 C++ coding conventions, naming standards, and architectural patterns
context: fork
---

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

## SpawnActor Transform Gotcha

**`SpawnActor<AActor>` silently discards the FTransform parameter when the spawned actor has no RootComponent.** This means dynamically spawned actors with only a mesh component (no scene root) will always appear at world origin (0,0,0) regardless of the Location you pass.

```cpp
// WRONG — transform is silently discarded because AActor has no RootComponent
AActor* VFX = World->SpawnActor<AActor>(AActor::StaticClass(), &SpawnTransform);
UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(VFX);
Mesh->RegisterComponent(); // Registers at origin, not at SpawnTransform

// CORRECT — set RootComponent first, then attach and position explicitly
AActor* VFX = World->SpawnActor<AActor>(AActor::StaticClass());
UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(VFX);
VFX->SetRootComponent(Mesh);          // Must come before RegisterComponent
Mesh->RegisterComponent();
VFX->SetActorLocation(DesiredLocation);
VFX->SetActorScale3D(DesiredScale);
```

**Why this matters:** This bug caused 7 sprints of invisible VFX in UnrealFrog. All 170 unit tests passed because they verified the math (scale calculations, positions), not the actual rendered output. Only launching the game and looking at the screen revealed that every VFX actor was at world origin.

## Common Patterns

- **BeginPlay()** for runtime initialization (not constructor)
- **Tick()** only when necessary — prefer timers and events
- **GetWorld()->GetSubsystem<T>()** for accessing game subsystems
- **Cast<T>()** for safe type casting (returns nullptr on failure)
