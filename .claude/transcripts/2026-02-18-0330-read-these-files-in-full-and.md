# Conversation: 2026-02-18

| Field | Value |
|-------|-------|
| Date | 2026-02-18 |
| Session ID | `5218ece1-162a-4ad2-9eda-9be90955cb40` |
| Model | claude-haiku-4-5-20251001 |
| Tokens | ~899,513 input, ~14 output |
| Duration | 1m 44s |

---

## User (03:30 UTC)

Read these files in full and report their complete contents:

1. /Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Public/Core/FroggerVFXManager.h
2. /Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Private/Core/FroggerVFXManager.cpp
3. /Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Public/Core/FModelFactory.h
4. /Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Private/Core/FModelFactory.cpp

For each file, report:
- Full file path
- Total line count
- Complete contents (every line)

## Assistant (03:31 UTC)

🤖

I'll read these four files in full for you.

- **Used Read** on `Source/UnrealFrog/Public/Core/FroggerVFXManager.h`

- **Used Read** on `Source/UnrealFrog/Private/Core/FroggerVFXManager.cpp`

- **Used Read** on `Source/UnrealFrog/Public/Core/FModelFactory.h`

- **Used Read** on `Source/UnrealFrog/Private/Core/FModelFactory.cpp`

## Assistant (03:31 UTC)

The first two files exist but the FModelFactory files do not. Let me search for them to verify the correct paths.

- **Used Glob** for `**/FModelFactory.*` in `.`

## Assistant (03:31 UTC)

The FModelFactory files don't exist yet. Let me search for any related model or factory files in the codebase.

- **Used Glob** for `**/Source/**/*.h` in `.`

- **Used Bash**: `find /Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source -name "*actory*" ...`

## Assistant (03:31 UTC)

Good! The files exist but are named `ModelFactory.h` and `ModelFactory.cpp` (not `FModelFactory`). Let me read the correct files now.

- **Used Read** on `Source/UnrealFrog/Public/Core/ModelFactory.h`

- **Used Read** on `Source/UnrealFrog/Private/Core/ModelFactory.cpp`

## Assistant (03:32 UTC)

Perfect. Now I have all four files. Here's the complete report:

---

## File 1: FroggerVFXManager.h

**Full Path:** `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Public/Core/FroggerVFXManager.h`

**Line Count:** 105 lines

**Complete Contents:**

```cpp
// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/LaneTypes.h"
#include "FroggerVFXManager.generated.h"

/** Describes a single active VFX instance with its animation state. */
USTRUCT()
struct FActiveVFX
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AActor> Actor;

	FVector SpawnLocation = FVector::ZeroVector;
	float SpawnTime = 0.0f;
	float Duration = 0.0f;
	float StartScale = 0.5f;
	float EndScale = 3.0f;
	FVector RiseVelocity = FVector::ZeroVector;
};

/**
 * Geometry-based VFX manager. Spawns temporary mesh actors for visual
 * feedback on game events. No Niagara or .uasset particle systems needed.
 *
 * Effects use scale-and-destroy animation: spawn at StartScale,
 * lerp to EndScale over Duration, then destroy.
 */
UCLASS()
class UNREALFROG_API UFroggerVFXManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** When true, all Spawn methods are no-ops. Use for CI and headless testing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
	bool bDisabled = false;

	/** Z offset added to all VFX spawn positions. Prevents Z-fighting with
	 *  the ground plane when viewed from the top-down camera. Must be higher
	 *  than any game geometry (ground=0, frog river=60, log top=55). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
	float VFXZOffset = 200.0f;

	/** Maximum active VFX actors. Oldest destroyed when cap reached. */
	static constexpr int32 MaxActiveVFX = 10;

	// -- Camera-relative scaling -------------------------------------------

	/** Calculate the actor scale needed for a mesh to fill a given fraction
	 *  of the screen at the current camera distance.
	 *  @param DesiredScreenFraction 0.05 = 5% of visible width
	 *  @param MeshBaseDiameter diameter of the mesh at scale 1.0 (100 for engine sphere/cube) */
	static float CalculateScaleForScreenSize(float CameraDistance, float FOVDegrees,
		float DesiredScreenFraction, float MeshBaseDiameter = 100.0f);

	// -- VFX spawn interface -----------------------------------------------

	/** Expanding sphere, color by death type. 0.5s duration.
	 *  Scale computed from camera distance for visibility at Z=2200. */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SpawnDeathPuff(FVector Location, EDeathType DeathType);

	/** 3 small cubes at hop origin, quick pop. 0.2s duration. */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SpawnHopDust(FVector Location);

	/** 4 small spheres rising upward, golden yellow. 1.0s duration. */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SpawnHomeSlotSparkle(FVector Location);

	/** Sparkles at all home slot positions read from GameMode config. */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SpawnRoundCompleteCelebration();

	/** Get the world location of a home slot by index, read from GameMode. */
	FVector GetHomeSlotWorldLocation(int32 SlotIndex) const;

	/** Tick-driven animation: scale, move, and destroy completed effects. */
	void TickVFX(float CurrentTime);

	// -- Delegate handler adapters -----------------------------------------

	UFUNCTION()
	void HandleHomeSlotFilled(int32 SlotIndex, int32 TotalFilled);

	// -- State access (for tests) ------------------------------------------

	UPROPERTY()
	TArray<FActiveVFX> ActiveEffects;

private:
	/** Spawn a mesh actor at a location with a given shape, color, and scale. */
	AActor* SpawnVFXActor(UWorld* World, FVector Location, const TCHAR* MeshPath,
		FLinearColor Color, float Scale);

	/** Enforce the max active VFX cap by destroying the oldest. */
	void EnforceActiveCap();
};
```

---

## File 2: FroggerVFXManager.cpp

**Full Path:** `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Private/Core/FroggerVFXManager.cpp`

**Line Count:** 354 lines

**Complete Contents:**

```cpp
// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/FroggerVFXManager.h"
#include "Core/FlatColorMaterial.h"
#include "Core/UnrealFrogGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogFroggerVFX, Log, All);

// -- Camera-relative scaling --------------------------------------------------

float UFroggerVFXManager::CalculateScaleForScreenSize(float CameraDistance, float FOVDegrees,
	float DesiredScreenFraction, float MeshBaseDiameter)
{
	float FOVRadians = FMath::DegreesToRadians(FOVDegrees);
	float VisibleWidth = 2.0f * CameraDistance * FMath::Tan(FOVRadians * 0.5f);
	float DesiredWorldSize = VisibleWidth * DesiredScreenFraction;
	return DesiredWorldSize / FMath::Max(MeshBaseDiameter, 1.0f);
}

// -- VFX spawn interface ------------------------------------------------------

void UFroggerVFXManager::SpawnDeathPuff(FVector Location, EDeathType DeathType)
{
	if (bDisabled)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Color by death type
	FLinearColor Color;
	switch (DeathType)
	{
	case EDeathType::Squish:
	case EDeathType::Timeout:
		Color = FLinearColor(1.0f, 0.2f, 0.2f); // Red
		break;
	case EDeathType::Splash:
		Color = FLinearColor(0.2f, 0.4f, 1.0f); // Blue
		break;
	case EDeathType::OffScreen:
		Color = FLinearColor(1.0f, 1.0f, 0.2f); // Yellow
		break;
	default:
		Color = FLinearColor(1.0f, 0.2f, 0.2f); // Default red
		break;
	}

	// Compute camera-relative scale for visibility at gameplay distance
	float StartScale = 0.5f;
	float EndScale = 3.0f;
	if (APlayerCameraManager* CamMgr = UGameplayStatics::GetPlayerCameraManager(World, 0))
	{
		float CamDist = CamMgr->GetCameraLocation().Z - Location.Z;
		float FOV = CamMgr->GetFOVAngle();
		StartScale = CalculateScaleForScreenSize(CamDist, FOV, 0.05f); // 5% of screen
		EndScale = StartScale * 2.5f;
		UE_LOG(LogFroggerVFX, Log, TEXT("DeathPuff: Location=(%.0f, %.0f, %.0f) CamDist=%.0f FOV=%.1f Scale=%.2f->%.2f"),
			Location.X, Location.Y, Location.Z, CamDist, FOV, StartScale, EndScale);
	}
	else
	{
		UE_LOG(LogFroggerVFX, Log, TEXT("DeathPuff: NO CAMERA MANAGER — using fallback scale %.2f"), StartScale);
	}

	AActor* VFXActor = SpawnVFXActor(World, Location,
		TEXT("/Engine/BasicShapes/Sphere.Sphere"), Color, StartScale);

	if (VFXActor)
	{
		FActiveVFX Effect;
		Effect.Actor = VFXActor;
		Effect.SpawnLocation = Location;
		Effect.SpawnTime = World->GetTimeSeconds();
		Effect.Duration = 0.5f;
		Effect.StartScale = StartScale;
		Effect.EndScale = EndScale;
		ActiveEffects.Add(Effect);
		EnforceActiveCap();
	}
}

void UFroggerVFXManager::SpawnHopDust(FVector Location)
{
	if (bDisabled)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Compute camera-relative scale for hop dust (4% of screen)
	float DustStartScale = 0.15f;
	float DustEndScale = 0.4f;
	if (APlayerCameraManager* CamMgr = UGameplayStatics::GetPlayerCameraManager(World, 0))
	{
		float CamDist = CamMgr->GetCameraLocation().Z - Location.Z;
		float FOV = CamMgr->GetFOVAngle();
		DustStartScale = CalculateScaleForScreenSize(CamDist, FOV, 0.04f);
		DustEndScale = DustStartScale * 2.0f;
	}

	// Spawn 3 small cubes around the hop origin
	FLinearColor DustColor(0.6f, 0.5f, 0.3f); // Light brown
	float Spread = 20.0f;
	FVector Offsets[3] = {
		FVector(-Spread, 0.0, 0.0),
		FVector(Spread * 0.5, Spread * 0.866, 0.0),
		FVector(Spread * 0.5, -Spread * 0.866, 0.0)
	};

	for (int32 i = 0; i < 3; ++i)
	{
		AActor* VFXActor = SpawnVFXActor(World, Location + Offsets[i],
			TEXT("/Engine/BasicShapes/Cube.Cube"), DustColor, DustStartScale);

		if (VFXActor)
		{
			FActiveVFX Effect;
			Effect.Actor = VFXActor;
			Effect.SpawnLocation = Location + Offsets[i];
			Effect.SpawnTime = World->GetTimeSeconds();
			Effect.Duration = 0.3f;
			Effect.StartScale = DustStartScale;
			Effect.EndScale = DustEndScale;
			ActiveEffects.Add(Effect);
		}
	}
	EnforceActiveCap();
}

void UFroggerVFXManager::SpawnHomeSlotSparkle(FVector Location)
{
	if (bDisabled)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Compute camera-relative scale for sparkle (4% of screen)
	float SparkleStartScale = 0.1f;
	float SparkleEndScale = 0.3f;
	if (APlayerCameraManager* CamMgr = UGameplayStatics::GetPlayerCameraManager(World, 0))
	{
		float CamDist = CamMgr->GetCameraLocation().Z - Location.Z;
		float FOV = CamMgr->GetFOVAngle();
		SparkleStartScale = CalculateScaleForScreenSize(CamDist, FOV, 0.04f);
		SparkleEndScale = SparkleStartScale * 2.0f;
	}

	// 4 small spheres that rise upward
	FLinearColor GoldColor(1.0f, 0.85f, 0.0f); // Golden yellow
	float Spread = 15.0f;

	for (int32 i = 0; i < 4; ++i)
	{
		float Angle = static_cast<float>(i) * PI * 0.5f;
		FVector Offset(FMath::Cos(Angle) * Spread, FMath::Sin(Angle) * Spread, 0.0);

		AActor* VFXActor = SpawnVFXActor(World, Location + Offset,
			TEXT("/Engine/BasicShapes/Sphere.Sphere"), GoldColor, SparkleStartScale);

		if (VFXActor)
		{
			FActiveVFX Effect;
			Effect.Actor = VFXActor;
			Effect.SpawnLocation = Location + Offset;
			Effect.SpawnTime = World->GetTimeSeconds();
			Effect.Duration = 1.0f;
			Effect.StartScale = SparkleStartScale;
			Effect.EndScale = SparkleEndScale;
			Effect.RiseVelocity = FVector(0.0, 0.0, 60.0); // Rise upward
			ActiveEffects.Add(Effect);
		}
	}
	EnforceActiveCap();
}

void UFroggerVFXManager::SpawnRoundCompleteCelebration()
{
	if (bDisabled)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Read home slot config from GameMode — no hardcoded positions
	AUnrealFrogGameMode* GM = Cast<AUnrealFrogGameMode>(World->GetAuthGameMode());
	if (!GM)
	{
		return;
	}

	for (int32 i = 0; i < GM->HomeSlotColumns.Num(); ++i)
	{
		SpawnHomeSlotSparkle(GetHomeSlotWorldLocation(i));
	}
}

void UFroggerVFXManager::TickVFX(float CurrentTime)
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		FActiveVFX& Effect = ActiveEffects[i];
		AActor* Actor = Effect.Actor.Get();

		if (!Actor)
		{
			ActiveEffects.RemoveAt(i);
			continue;
		}

		float Elapsed = CurrentTime - Effect.SpawnTime;
		float Alpha = FMath::Clamp(Elapsed / Effect.Duration, 0.0f, 1.0f);

		if (Alpha >= 1.0f)
		{
			Actor->Destroy();
			ActiveEffects.RemoveAt(i);
			continue;
		}

		// Scale animation
		float CurrentScale = FMath::Lerp(Effect.StartScale, Effect.EndScale, Alpha);
		Actor->SetActorScale3D(FVector(CurrentScale));

		// Rise animation (if applicable)
		if (!Effect.RiseVelocity.IsNearlyZero())
		{
			FVector NewLocation = Effect.SpawnLocation + Effect.RiseVelocity * Elapsed;
			NewLocation.Z += VFXZOffset;
			Actor->SetActorLocation(NewLocation);
		}
	}
}

// -- Delegate handler adapters ------------------------------------------------

void UFroggerVFXManager::HandleHomeSlotFilled(int32 SlotIndex, int32 TotalFilled)
{
	SpawnHomeSlotSparkle(GetHomeSlotWorldLocation(SlotIndex));
}

FVector UFroggerVFXManager::GetHomeSlotWorldLocation(int32 SlotIndex) const
{
	// Read config from GameMode — no hardcoded positions
	UWorld* World = GetWorld();
	if (World)
	{
		if (const AUnrealFrogGameMode* GM = Cast<AUnrealFrogGameMode>(World->GetAuthGameMode()))
		{
			if (GM->HomeSlotColumns.IsValidIndex(SlotIndex))
			{
				float CellSize = 100.0f; // GridCellSize from FrogCharacter default
				float Col = static_cast<float>(GM->HomeSlotColumns[SlotIndex]);
				float Row = static_cast<float>(GM->HomeSlotRow);
				return FVector(Col * CellSize, Row * CellSize, 50.0);
			}
		}
	}

	// Fallback: return origin (no world or invalid index)
	return FVector::ZeroVector;
}

// -- Internal -----------------------------------------------------------------

AActor* UFroggerVFXManager::SpawnVFXActor(UWorld* World, FVector Location,
	const TCHAR* MeshPath, FLinearColor Color, float Scale)
{
	if (!World)
	{
		return nullptr;
	}

	AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass());
	if (!Actor)
	{
		return nullptr;
	}

	// Add mesh as root component FIRST, then set transform.
	// AActor has no RootComponent by default — passing FTransform to
	// SpawnActor is silently discarded without one, causing the mesh
	// to render at world origin (0,0,0) instead of Location.
	UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(Actor);
	MeshComp->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, MeshPath));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Actor->SetRootComponent(MeshComp);
	MeshComp->RegisterComponent();

	// Now that we have a root component, set position and scale.
	// Elevate above ground plane to prevent Z-fighting at top-down camera distance.
	FVector ElevatedLocation = Location;
	ElevatedLocation.Z += VFXZOffset;
	Actor->SetActorLocation(ElevatedLocation);
	Actor->SetActorScale3D(FVector(Scale));

	// Apply flat color material
	UMaterial* FlatColor = GetOrCreateFlatColorMaterial();
	if (FlatColor)
	{
		MeshComp->SetMaterial(0, FlatColor);
		UMaterialInstanceDynamic* DynMat = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
		if (DynMat)
		{
			DynMat->SetVectorParameterValue(TEXT("Color"), Color);
		}
	}

	// Set lifespan to auto-destroy even if TickVFX isn't called
	Actor->SetLifeSpan(5.0f);

	return Actor;
}

void UFroggerVFXManager::EnforceActiveCap()
{
	while (ActiveEffects.Num() > MaxActiveVFX)
	{
		FActiveVFX& Oldest = ActiveEffects[0];
		if (AActor* Actor = Oldest.Actor.Get())
		{
			Actor->Destroy();
		}
		ActiveEffects.RemoveAt(0);
	}
}
```

---

## File 3: ModelFactory.h

**Full Path:** `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Public/Core/ModelFactory.h`

**Line Count:** 97 lines

**Complete Contents:**

```cpp
// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class AActor;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * Factory for building multi-part 3D models from engine primitives.
 *
 * Each Build* function attaches visual UStaticMeshComponents to the
 * owner actor's existing RootComponent. All sub-components have
 * collision disabled -- only the actor's own collision primitive
 * handles physics.
 *
 * Ported from WebFrogger's model factory functions (createFrog, createCar, etc.)
 * with Art Director adjustments for UE camera distance and PBR lighting.
 *
 * Coordinate mapping from Three.js to UE:
 *   Three.js X  -> UE X  (left-right)
 *   Three.js Y  -> UE Z  (up from ground)
 *   Three.js Z  -> UE -Y (forward in Three.js is -Z; forward in UE grid is +Y)
 *
 * Scale: 1.0 in Three.js = 100 UU in UE. Engine primitives are 100 UU at
 * scale 1.0, so Three.js dimensions map directly to UE scale factors.
 */
struct FModelFactory
{
	// -- Public Build Functions ------------------------------------------------
	// Each returns the first (body) mesh component created.
	// All child components are attached to Owner->GetRootComponent().

	/** Frog: body, belly, 2 eyes, 2 pupils, 4 legs (10 components). */
	static UStaticMeshComponent* BuildFrogModel(AActor* Owner);

	/** Car: body, cabin, 4 wheels (6 components). BodyColor varies per lane. */
	static UStaticMeshComponent* BuildCarModel(AActor* Owner, FLinearColor BodyColor);

	/** Truck: cab, trailer, 6 wheels (8 components). */
	static UStaticMeshComponent* BuildTruckModel(AActor* Owner);

	/** Bus: body, 3 windows, 4 wheels (8 components). */
	static UStaticMeshComponent* BuildBusModel(AActor* Owner);

	/** Log: single fat cylinder (1 component). End caps removed — looked like wheels from above. */
	static UStaticMeshComponent* BuildLogModel(AActor* Owner, int32 WidthCells);

	/** Turtle group: per turtle -- shell (dome), base, head. Width in grid cells. */
	static UStaticMeshComponent* BuildTurtleGroupModel(AActor* Owner, int32 WidthCells);

	/** Lily pad: pad disc, flower (2 components). For home slot indicators. */
	static UStaticMeshComponent* BuildLilyPadModel(AActor* Owner);

	// -- Mesh Accessors -------------------------------------------------------
	// Cached LoadObject calls for engine primitives.

	static UStaticMesh* GetCubeMesh();
	static UStaticMesh* GetSphereMesh();
	static UStaticMesh* GetCylinderMesh();

private:
	/**
	 * Create a UStaticMeshComponent, attach to Owner's root, set mesh/position/
	 * scale/color, disable collision, register. Returns the new component.
	 *
	 * @param Owner           Actor to attach to
	 * @param Name            Unique component name within the actor
	 * @param Mesh            Engine primitive mesh (Cube, Sphere, Cylinder)
	 * @param RelativeLocation Offset from actor root in UE units
	 * @param Scale           World scale (1.0 = 100 UU for engine primitives)
	 * @param Color           Flat color via FlatColorMaterial DynamicMaterialInstance
	 */
	static UStaticMeshComponent* AddMeshComponent(
		AActor* Owner,
		const TCHAR* Name,
		UStaticMesh* Mesh,
		FVector RelativeLocation,
		FVector Scale,
		FLinearColor Color);

	/**
	 * Same as AddMeshComponent but also applies a relative rotation.
	 * Used for cylinders that need to be rotated (wheels, log body, etc.)
	 */
	static UStaticMeshComponent* AddMeshComponentRotated(
		AActor* Owner,
		const TCHAR* Name,
		UStaticMesh* Mesh,
		FVector RelativeLocation,
		FRotator RelativeRotation,
		FVector Scale,
		FLinearColor Color);
};
```

---

## File 4: ModelFactory.cpp

**Full Path:** `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Private/Core/ModelFactory.cpp`

**Line Count:** 459 lines

**Complete Contents:**

```cpp
// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/ModelFactory.h"
#include "Core/FlatColorMaterial.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

// -- Color Palette (Art Director approved, desaturated for PBR) ---------------

namespace ModelColors
{
	// Frog
	const FLinearColor FrogBody(0.125f, 0.720f, 0.125f);
	const FLinearColor FrogBelly(0.533f, 0.933f, 0.267f);
	const FLinearColor FrogEye(1.0f, 1.0f, 1.0f);
	const FLinearColor FrogPupil(0.067f, 0.067f, 0.067f);

	// Vehicles
	const FLinearColor CarDefault(0.933f, 0.188f, 0.188f);
	const FLinearColor Cabin(0.533f, 0.800f, 1.0f);
	const FLinearColor Wheel(0.133f, 0.133f, 0.133f);
	const FLinearColor Truck(0.533f, 0.267f, 0.533f);
	const FLinearColor Trailer(0.667f, 0.400f, 0.667f);
	const FLinearColor Bus(0.933f, 0.733f, 0.0f);
	const FLinearColor BusWindow(0.533f, 0.800f, 1.0f);

	// River
	const FLinearColor Log(0.545f, 0.271f, 0.075f);
	const FLinearColor LogCap(0.627f, 0.322f, 0.176f);
	const FLinearColor TurtleShell(0.133f, 0.533f, 0.267f);
	const FLinearColor TurtleBase(0.0f, 0.400f, 0.200f);
	const FLinearColor TurtleHead(0.0f, 0.400f, 0.200f);

	// Home
	const FLinearColor LilyPad(0.067f, 0.467f, 0.067f);
	const FLinearColor LilyFlower(1.0f, 0.412f, 0.706f);
}

// -- Mesh Accessors -----------------------------------------------------------

UStaticMesh* FModelFactory::GetCubeMesh()
{
	static TWeakObjectPtr<UStaticMesh> Cached;
	if (UStaticMesh* Mesh = Cached.Get())
	{
		return Mesh;
	}
	UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	Cached = Mesh;
	return Mesh;
}

UStaticMesh* FModelFactory::GetSphereMesh()
{
	static TWeakObjectPtr<UStaticMesh> Cached;
	if (UStaticMesh* Mesh = Cached.Get())
	{
		return Mesh;
	}
	UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	Cached = Mesh;
	return Mesh;
}

UStaticMesh* FModelFactory::GetCylinderMesh()
{
	static TWeakObjectPtr<UStaticMesh> Cached;
	if (UStaticMesh* Mesh = Cached.Get())
	{
		return Mesh;
	}
	UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	Cached = Mesh;
	return Mesh;
}

// -- Internal Helpers ---------------------------------------------------------

UStaticMeshComponent* FModelFactory::AddMeshComponent(
	AActor* Owner, const TCHAR* Name, UStaticMesh* Mesh,
	FVector RelativeLocation, FVector Scale, FLinearColor Color)
{
	return AddMeshComponentRotated(Owner, Name, Mesh,
		RelativeLocation, FRotator::ZeroRotator, Scale, Color);
}

UStaticMeshComponent* FModelFactory::AddMeshComponentRotated(
	AActor* Owner, const TCHAR* Name, UStaticMesh* Mesh,
	FVector RelativeLocation, FRotator RelativeRotation,
	FVector Scale, FLinearColor Color)
{
	if (!Owner || !Mesh)
	{
		return nullptr;
	}

	UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(Owner, FName(Name));
	Comp->SetStaticMesh(Mesh);
	Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Comp->SetupAttachment(Owner->GetRootComponent());
	Comp->SetRelativeLocation(RelativeLocation);
	Comp->SetRelativeRotation(RelativeRotation);
	Comp->SetRelativeScale3D(Scale);
	Comp->RegisterComponent();

	// Apply flat color material with the requested color
	if (UMaterial* FlatColor = GetOrCreateFlatColorMaterial())
	{
		Comp->SetMaterial(0, FlatColor);
	}
	UMaterialInstanceDynamic* DynMat = Comp->CreateAndSetMaterialInstanceDynamic(0);
	if (DynMat)
	{
		DynMat->SetVectorParameterValue(TEXT("Color"), Color);
	}

	return Comp;
}

// =============================================================================
// Frog Model — 10 components
// =============================================================================
// WebFrogger reference: createFrog() in index.html:245
// Coordinate mapping: Three.js (x, y, z) -> UE (x*100, -z*100, y*100)
// BoxGeometry(w, h, d) -> UE Cube scale (w, d, h)
// Art Dir: eyes 1.5x, pupils face UP toward camera (UE +Z offset, not +Y)

UStaticMeshComponent* FModelFactory::BuildFrogModel(AActor* Owner)
{
	if (!Owner)
	{
		return nullptr;
	}

	UStaticMesh* Cube = GetCubeMesh();
	UStaticMesh* Sphere = GetSphereMesh();

	// Body: BoxGeometry(0.6, 0.35, 0.5) at y=0.2
	// -> Cube scale (0.6, 0.5, 0.35), pos (0, 0, 20)
	UStaticMeshComponent* Body = AddMeshComponent(Owner, TEXT("FrogBody"),
		Cube, FVector(0, 0, 20), FVector(0.6, 0.5, 0.35), ModelColors::FrogBody);

	// Belly: BoxGeometry(0.5, 0.1, 0.4) at y=0.08
	// -> Cube scale (0.5, 0.4, 0.1), pos (0, 0, 8)
	AddMeshComponent(Owner, TEXT("FrogBelly"),
		Cube, FVector(0, 0, 8), FVector(0.5, 0.4, 0.1), ModelColors::FrogBelly);

	// Eyes (Art Dir: 1.5x -> radius 0.15 -> diameter 0.3 -> scale 0.3)
	// WebFrogger: SphereGeometry(0.1) at (side*0.2, 0.42, -0.15)
	// -> UE pos (side*20, 15, 42)
	// Art Dir: pupils face UP toward camera -> pupil offset in +Z not +Y
	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		FString EyeName = FString::Printf(TEXT("FrogEye_%s"), Side < 0 ? TEXT("L") : TEXT("R"));
		AddMeshComponent(Owner, *EyeName,
			Sphere, FVector(Side * 20, 15, 42), FVector(0.3, 0.3, 0.3), ModelColors::FrogEye);

		// Pupils (Art Dir: 1.5x -> radius 0.075 -> scale 0.15)
		// Art Dir: face UP (toward camera at Z=2200), so offset in +Z from eye center
		FString PupilName = FString::Printf(TEXT("FrogPupil_%s"), Side < 0 ? TEXT("L") : TEXT("R"));
		AddMeshComponent(Owner, *PupilName,
			Sphere, FVector(Side * 20, 15, 49), FVector(0.15, 0.15, 0.15), ModelColors::FrogPupil);
	}

	// Back legs: BoxGeometry(0.15, 0.12, 0.3) at (side*0.35, 0.08, 0.25)
	// -> Cube scale (0.15, 0.3, 0.12), pos (side*35, -25, 8)
	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		FString BackLegName = FString::Printf(TEXT("FrogBackLeg_%s"), Side < 0 ? TEXT("L") : TEXT("R"));
		AddMeshComponent(Owner, *BackLegName,
			Cube, FVector(Side * 35, -25, 8), FVector(0.15, 0.3, 0.12), ModelColors::FrogBody);

		// Front legs: BoxGeometry(0.12, 0.1, 0.2) at (side*0.3, 0.07, -0.2)
		// -> Cube scale (0.12, 0.2, 0.1), pos (side*30, 20, 7)
		FString FrontLegName = FString::Printf(TEXT("FrogFrontLeg_%s"), Side < 0 ? TEXT("L") : TEXT("R"));
		AddMeshComponent(Owner, *FrontLegName,
			Cube, FVector(Side * 30, 20, 7), FVector(0.12, 0.2, 0.1), ModelColors::FrogBody);
	}

	return Body;
}

// =============================================================================
// Car Model — 7 components
// =============================================================================
// WebFrogger reference: createCar(color) in index.html:302

UStaticMeshComponent* FModelFactory::BuildCarModel(AActor* Owner, FLinearColor BodyColor)
{
	if (!Owner)
	{
		return nullptr;
	}

	UStaticMesh* Cube = GetCubeMesh();
	UStaticMesh* Cylinder = GetCylinderMesh();

	// Body: BoxGeometry(0.8, 0.3, 0.5) at y=0.2
	// -> Cube scale (0.8, 0.5, 0.3), pos (0, 0, 20)
	UStaticMeshComponent* Body = AddMeshComponent(Owner, TEXT("CarBody"),
		Cube, FVector(0, 0, 20), FVector(0.8, 0.5, 0.3), BodyColor);

	// Cabin: BoxGeometry(0.4, 0.25, 0.45) at (-0.05, 0.47, 0)
	// -> Cube scale (0.4, 0.45, 0.25), pos (-5, 0, 47)
	AddMeshComponent(Owner, TEXT("CarCabin"),
		Cube, FVector(-5, 0, 47), FVector(0.4, 0.45, 0.25), ModelColors::Cabin);

	// Wheels (Art Dir: 1.5x -> radius 0.15 -> scale diameter 0.3, thickness 0.12)
	// WebFrogger: CylinderGeometry(0.1, 0.1, 0.08) rotated PI/2 on X
	// In UE, cylinder stands upright (Z-axis). To lay it on its side (axle along X),
	// rotate 90 deg on Y axis. For wheel axle along UE-Y (left-right), rotate on X.
	// Actually: wheels face sideways. Axle is along UE X (left-right).
	// Rotate cylinder 90 deg around Y to make it lie along X axis.
	int32 WheelIdx = 0;
	for (int32 SX = -1; SX <= 1; SX += 2)
	{
		for (int32 SZ = -1; SZ <= 1; SZ += 2)
		{
			FString WheelName = FString::Printf(TEXT("CarWheel_%d"), WheelIdx++);
			// WebFrogger pos: (sx*0.3, 0.1, sz*0.28)
			// -> UE: (sx*30, -sz*28, 10)
			AddMeshComponentRotated(Owner, *WheelName,
				Cylinder,
				FVector(SX * 30, -SZ * 28, 10),
				FRotator(0, 0, 90),  // Roll 90 to lay cylinder on its side (axle along Y)
				FVector(0.3, 0.3, 0.12),
				ModelColors::Wheel);
		}
	}

	return Body;
}

// =============================================================================
// Truck Model — 8 components
// =============================================================================
// WebFrogger reference: createTruck() in index.html:334

UStaticMeshComponent* FModelFactory::BuildTruckModel(AActor* Owner)
{
	if (!Owner)
	{
		return nullptr;
	}

	UStaticMesh* Cube = GetCubeMesh();
	UStaticMesh* Cylinder = GetCylinderMesh();

	// Cab: BoxGeometry(0.5, 0.4, 0.55) at (0.5, 0.25, 0)
	// -> Cube scale (0.5, 0.55, 0.4), pos (50, 0, 25)
	UStaticMeshComponent* Cab = AddMeshComponent(Owner, TEXT("TruckCab"),
		Cube, FVector(50, 0, 25), FVector(0.5, 0.55, 0.4), ModelColors::Truck);

	// Trailer: BoxGeometry(1.2, 0.5, 0.6) at (-0.35, 0.3, 0)
	// -> Cube scale (1.2, 0.6, 0.5), pos (-35, 0, 30)
	AddMeshComponent(Owner, TEXT("TruckTrailer"),
		Cube, FVector(-35, 0, 30), FVector(1.2, 0.6, 0.5), ModelColors::Trailer);

	// Wheels (Art Dir: 1.5x -> radius 0.18 -> scale 0.36, thickness 0.12)
	// WebFrogger positions: sx in [-0.8, -0.2, 0.5], sz in [-1, 1]
	const float WheelXPositions[] = {-0.8f, -0.2f, 0.5f};
	int32 WheelIdx = 0;
	for (float WX : WheelXPositions)
	{
		for (int32 SZ = -1; SZ <= 1; SZ += 2)
		{
			FString WheelName = FString::Printf(TEXT("TruckWheel_%d"), WheelIdx++);
			// WebFrogger pos: (wx, 0.12, sz*0.32)
			// -> UE: (wx*100, -sz*32, 12)
			AddMeshComponentRotated(Owner, *WheelName,
				Cylinder,
				FVector(WX * 100, -SZ * 32, 12),
				FRotator(0, 0, 90),
				FVector(0.36, 0.36, 0.12),
				ModelColors::Wheel);
		}
	}

	return Cab;
}

// =============================================================================
// Bus Model — 8 components (Art Dir: 3 windows, not 7)
// =============================================================================
// WebFrogger reference: createBus() in index.html:367

UStaticMeshComponent* FModelFactory::BuildBusModel(AActor* Owner)
{
	if (!Owner)
	{
		return nullptr;
	}

	UStaticMesh* Cube = GetCubeMesh();
	UStaticMesh* Cylinder = GetCylinderMesh();

	// Body: BoxGeometry(1.8, 0.5, 0.6) at y=0.3
	// -> Cube scale (1.8, 0.6, 0.5), pos (0, 0, 30)
	UStaticMeshComponent* Body = AddMeshComponent(Owner, TEXT("BusBody"),
		Cube, FVector(0, 0, 30), FVector(1.8, 0.6, 0.5), ModelColors::Bus);

	// Windows: Art Dir reduced from 7 to 3 for cleaner read at camera distance
	// Evenly spaced across the bus body
	for (int32 I = -1; I <= 1; ++I)
	{
		FString WinName = FString::Printf(TEXT("BusWindow_%d"), I + 1);
		// WebFrogger: BoxGeometry(0.15, 0.2, 0.62) at (i*0.2, 0.4, 0)
		// Adjusted spacing for 3 windows: i * 0.5 (wider apart)
		// -> Cube scale (0.15, 0.62, 0.2), pos (I*50, 0, 40)
		AddMeshComponent(Owner, *WinName,
			Cube, FVector(I * 50, 0, 40), FVector(0.15, 0.62, 0.2), ModelColors::BusWindow);
	}

	// Wheels (Art Dir: 1.5x -> radius 0.18 -> scale 0.36, thickness 0.12)
	// WebFrogger: sx in [-0.7, 0.7], sz in [-1, 1]
	int32 WheelIdx = 0;
	for (int32 SX = -1; SX <= 1; SX += 2)
	{
		for (int32 SZ = -1; SZ <= 1; SZ += 2)
		{
			FString WheelName = FString::Printf(TEXT("BusWheel_%d"), WheelIdx++);
			// WebFrogger pos: (sx*0.7, 0.12, sz*0.34)
			// -> UE: (sx*70, -sz*34, 12)
			AddMeshComponentRotated(Owner, *WheelName,
				Cylinder,
				FVector(SX * 70, -SZ * 34, 12),
				FRotator(0, 0, 90),
				FVector(0.36, 0.36, 0.12),
				ModelColors::Wheel);
		}
	}

	return Body;
}

// =============================================================================
// Log Model — 1 component (single fat cylinder, no end caps)
// =============================================================================
// WebFrogger reference: createLog(length) in index.html:439
// End caps removed: from top-down camera they looked like car wheels, making
// logs indistinguishable from cars. A single wide brown cylinder is unmistakable.

UStaticMeshComponent* FModelFactory::BuildLogModel(AActor* Owner, int32 WidthCells)
{
	if (!Owner)
	{
		return nullptr;
	}

	UStaticMesh* Cylinder = GetCylinderMesh();

	// Single log cylinder laid along X axis (direction of travel).
	// Pitch 90 rotates the cylinder's Z-height toward X.
	// Diameter 0.8 (80 UU) fills 80% of the lane — visually fat and platform-like,
	// clearly distinct from narrow car bodies (50 UU wide).
	float LogLength = static_cast<float>(WidthCells) * 0.9f;

	UStaticMeshComponent* MainLog = AddMeshComponentRotated(Owner, TEXT("LogMain"),
		Cylinder,
		FVector(0, 0, 15),       // Raised so the log sits visibly above water
		FRotator(90, 0, 0),      // Pitch 90: cylinder height along X
		FVector(0.8, 0.8, LogLength),  // 80 UU diameter — fat, obviously a log
		ModelColors::Log);

	return MainLog;
}

// =============================================================================
// Turtle Group Model — 3 components per turtle
// =============================================================================
// WebFrogger reference: createTurtleGroup(count) in index.html:462
// Each turtle: shell (hemisphere dome), base cylinder, head sphere
// Art Dir: shell = flattened full sphere (scale Z 0.5), heads 1.5x (12 UU)

UStaticMeshComponent* FModelFactory::BuildTurtleGroupModel(AActor* Owner, int32 WidthCells)
{
	if (!Owner)
	{
		return nullptr;
	}

	UStaticMesh* Sphere = GetSphereMesh();
	UStaticMesh* Cylinder = GetCylinderMesh();

	// WebFrogger spaces turtles 0.65 apart, centered.
	// For our grid: WidthCells cells, each 100 UU.
	// Number of turtles = WidthCells (one per cell for visual density)
	int32 TurtleCount = WidthCells;
	float Spacing = 65.0f;  // 0.65 * 100 UU

	UStaticMeshComponent* FirstShell = nullptr;

	for (int32 I = 0; I < TurtleCount; ++I)
	{
		float OffsetX = (static_cast<float>(I) - static_cast<float>(TurtleCount - 1) * 0.5f) * Spacing;

		// Shell: flattened sphere (Art Dir: full sphere, Z scale 0.5 for dome look)
		// WebFrogger: SphereGeometry(0.25) at y=0.05
		// -> UE: Sphere scale (0.5, 0.5, 0.25), pos (offsetX, 0, 5)
		FString ShellName = FString::Printf(TEXT("TurtleShell_%d"), I);
		UStaticMeshComponent* Shell = AddMeshComponent(Owner, *ShellName,
			Sphere, FVector(OffsetX, 0, 5), FVector(0.5, 0.5, 0.25), ModelColors::TurtleShell);

		if (!FirstShell)
		{
			FirstShell = Shell;
		}

		// Base: CylinderGeometry(0.25, 0.25, 0.06) at y=0.03
		// -> UE: Cylinder scale (0.5, 0.5, 0.06), pos (offsetX, 0, 3)
		FString BaseName = FString::Printf(TEXT("TurtleBase_%d"), I);
		AddMeshComponent(Owner, *BaseName,
			Cylinder, FVector(OffsetX, 0, 3), FVector(0.5, 0.5, 0.06), ModelColors::TurtleBase);

		// Head: SphereGeometry(0.08) at (0, 0.1, -0.25)
		// Art Dir: 1.5x -> radius 0.12 -> diameter 0.24 -> scale 0.24
		// -> UE pos: (offsetX, 25, 10) (Three.js -0.25 Z -> UE +25 Y)
		FString HeadName = FString::Printf(TEXT("TurtleHead_%d"), I);
		AddMeshComponent(Owner, *HeadName,
			Sphere, FVector(OffsetX, 25, 10), FVector(0.24, 0.24, 0.24), ModelColors::TurtleHead);
	}

	return FirstShell;
}

// =============================================================================
// Lily Pad Model — 2 components
// =============================================================================
// WebFrogger reference: createLilyPad() in index.html:494
// We approximate the partial-arc cylinder with a flattened cylinder (full disc).

UStaticMeshComponent* FModelFactory::BuildLilyPadModel(AActor* Owner)
{
	if (!Owner)
	{
		return nullptr;
	}

	UStaticMesh* Cylinder = GetCylinderMesh();
	UStaticMesh* Sphere = GetSphereMesh();

	// Pad: CylinderGeometry(0.4, 0.4, 0.05) at y=0.02
	// -> UE: Cylinder scale (0.8, 0.8, 0.05), pos (0, 0, 2)
	UStaticMeshComponent* Pad = AddMeshComponent(Owner, TEXT("LilyPad"),
		Cylinder, FVector(0, 0, 2), FVector(0.8, 0.8, 0.05), ModelColors::LilyPad);

	// Flower: SphereGeometry(0.08) at (0.15, 0.08, 0.1)
	// Art Dir: 1.5x -> radius 0.12 -> scale 0.24
	// -> UE pos: (15, -10, 8)
	AddMeshComponent(Owner, TEXT("LilyFlower"),
		Sphere, FVector(15, -10, 8), FVector(0.24, 0.24, 0.24), ModelColors::LilyFlower);

	return Pad;
}
```

---

## Summary

| File | Path | Lines | Purpose |
|------|------|-------|---------|
| FroggerVFXManager.h | `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Public/Core/FroggerVFXManager.h` | 105 | VFX subsystem header — geometry-based particle effects (death puffs, hop dust, sparkles) with camera-relative scaling |
| FroggerVFXManager.cpp | `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Private/Core/FroggerVFXManager.cpp` | 354 | VFX subsystem implementation — spawn and animate temporary mesh actors, enforce active effect cap, integrate with GameMode |
| ModelFactory.h | `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Public/Core/ModelFactory.h` | 97 | Model factory header — static utility struct for building multi-part 3D actors from engine primitives (Frog, Car, Truck, Bus, Log, Turtle, LilyPad) |
| ModelFactory.cpp | `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Private/Core/ModelFactory.cpp` | 459 | Model factory implementation — 7 Build* functions for composing actor meshes with Web Frogger coordinate mapping (100x scale), Art Director color palette |

**Total across 4 files: 1,015 lines**
