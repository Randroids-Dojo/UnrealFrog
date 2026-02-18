# Conversation: 2026-02-18

| Field | Value |
|-------|-------|
| Date | 2026-02-18 |
| Session ID | `5218ece1-162a-4ad2-9eda-9be90955cb40` |
| Model | claude-haiku-4-5-20251001 |
| Tokens | ~362,376 input, ~6 output |
| Duration | 47s |

---

## User (03:33 UTC)

Read these files completely and report their full contents:

1. /Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Public/Core/LaneTypes.h
2. /Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Private/Core/HazardBase.cpp
3. /Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Private/Core/LaneManager.cpp
4. /Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Public/Core/HazardBase.h

Report every line of every file.

## Assistant (03:33 UTC)

🤖️

I'll read all four files for you.

- **Used Read** on `Source/UnrealFrog/Public/Core/LaneTypes.h`

- **Used Read** on `Source/UnrealFrog/Private/Core/HazardBase.cpp`

- **Used Read** on `Source/UnrealFrog/Private/Core/LaneManager.cpp`

- **Used Read** on `Source/UnrealFrog/Public/Core/HazardBase.h`

## Assistant (03:34 UTC)

## File 1: LaneTypes.h

**Lines 1-70** — Complete file

```cpp
// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LaneTypes.generated.h"

UENUM(BlueprintType)
enum class EDeathType : uint8
{
	None		UMETA(DisplayName = "None"),
	Squish		UMETA(DisplayName = "Squish"),
	Splash		UMETA(DisplayName = "Splash"),
	Timeout		UMETA(DisplayName = "Timeout"),
	OffScreen	UMETA(DisplayName = "Off Screen")
};

UENUM(BlueprintType)
enum class ELaneType : uint8
{
	Safe	UMETA(DisplayName = "Safe"),
	Road	UMETA(DisplayName = "Road"),
	River	UMETA(DisplayName = "River"),
	Goal	UMETA(DisplayName = "Goal")
};

UENUM(BlueprintType)
enum class EHazardType : uint8
{
	Car				UMETA(DisplayName = "Car"),
	Truck			UMETA(DisplayName = "Truck"),
	Bus				UMETA(DisplayName = "Bus"),
	Motorcycle		UMETA(DisplayName = "Motorcycle"),
	SmallLog		UMETA(DisplayName = "Small Log"),
	LargeLog		UMETA(DisplayName = "Large Log"),
	TurtleGroup		UMETA(DisplayName = "Turtle Group")
};

USTRUCT(BlueprintType)
struct FLaneConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
	ELaneType LaneType = ELaneType::Safe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
	EHazardType HazardType = EHazardType::Car;

	/** Movement speed in Unreal Units per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
	float Speed = 0.0f;

	/** Width of each hazard in grid cells */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
	int32 HazardWidth = 1;

	/** Minimum gap between hazards in grid cells */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
	int32 MinGapCells = 1;

	/** True = moves in +X direction, False = moves in -X direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
	bool bMovesRight = true;

	/** Which row this lane occupies (0-14) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
	int32 RowIndex = 0;
};
```

---

## File 2: HazardBase.cpp

**Lines 1-228** — Complete file

```cpp
// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/HazardBase.h"
#include "Core/FlatColorMaterial.h"
#include "Core/ModelFactory.h"
#include "Core/UnrealFrogGameMode.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AHazardBase::AHazardBase()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
	CollisionBox->SetGenerateOverlapEvents(true);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Overlap);
	SetRootComponent(CollisionBox);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionBox);

	// Pre-load both placeholder meshes -- actual assignment happens in BeginPlay
	// based on HazardType, but the mesh objects need to be found at construction time.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		CubeMeshAsset = CubeMesh.Object;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		CylinderMeshAsset = CylinderMesh.Object;
	}

}

void AHazardBase::BeginPlay()
{
	Super::BeginPlay();

	// NOTE: SetupMeshForHazardType() is NOT called here.
	// SpawnActor triggers BeginPlay immediately, but InitFromConfig
	// (which sets HazardType) is called AFTER SpawnActor returns.
	// SetupMeshForHazardType() is called at the end of InitFromConfig.
}

void AHazardBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Stop all movement and timers while game is paused
	if (UWorld* World = GetWorld())
	{
		if (AUnrealFrogGameMode* GM = Cast<AUnrealFrogGameMode>(World->GetAuthGameMode()))
		{
			if (GM->CurrentState == EGameState::Paused)
			{
				return;
			}
		}
	}

	TickMovement(DeltaTime);

	if (HazardType == EHazardType::TurtleGroup)
	{
		TickSubmerge(DeltaTime);
	}
}

void AHazardBase::TickMovement(float DeltaTime)
{
	FVector Location = GetActorLocation();

	float Direction = bMovesRight ? 1.0f : -1.0f;
	Location.X += Speed * Direction * DeltaTime;

	SetActorLocation(Location);

	WrapPosition();
}

void AHazardBase::TickSubmerge(float DeltaTime)
{
	// Wave 1: turtles are always solid, never submerge
	if (CurrentWave <= 1)
	{
		return;
	}

	SubmergeTimer += DeltaTime;

	switch (SubmergePhase)
	{
	case ESubmergePhase::Surface:
		if (SubmergeTimer >= SurfaceDuration)
		{
			SubmergePhase = ESubmergePhase::Warning;
			SubmergeTimer = 0.0f;
		}
		break;

	case ESubmergePhase::Warning:
		if (SubmergeTimer >= WarningDuration)
		{
			SubmergePhase = ESubmergePhase::Submerged;
			bIsSubmerged = true;
			SubmergeTimer = 0.0f;
		}
		break;

	case ESubmergePhase::Submerged:
		if (SubmergeTimer >= SubmergeDuration)
		{
			SubmergePhase = ESubmergePhase::Surface;
			bIsSubmerged = false;
			SubmergeTimer = 0.0f;
		}
		break;
	}
}

void AHazardBase::InitFromConfig(const FLaneConfig& Config, float InGridCellSize, int32 InGridColumns)
{
	Speed = Config.Speed;
	BaseSpeed = Config.Speed;
	HazardWidthCells = Config.HazardWidth;
	bMovesRight = Config.bMovesRight;
	HazardType = Config.HazardType;
	GridCellSize = InGridCellSize;
	GridColumns = InGridColumns;

	// River objects are rideable (logs and turtles)
	bIsRideable = (Config.LaneType == ELaneType::River);

	// Scale the collision box to match hazard width
	float HalfWidth = GetWorldWidth() * 0.5f;
	float HalfCell = GridCellSize * 0.5f;
	CollisionBox->SetBoxExtent(FVector(HalfWidth, HalfCell, HalfCell));

	// Now that HazardType is set, build the correct visual model.
	// This MUST happen here, not in BeginPlay — SpawnActor triggers
	// BeginPlay before InitFromConfig is called.
	SetupMeshForHazardType();
}

void AHazardBase::WrapPosition()
{
	FVector Location = GetActorLocation();
	float WorldWidth = GetWorldWidth();
	float GridWorldWidth = static_cast<float>(GridColumns) * GridCellSize;

	// Buffer: one hazard width past the boundary before wrapping
	float WrapMin = -WorldWidth;
	float WrapMax = GridWorldWidth + WorldWidth;

	if (bMovesRight && Location.X > WrapMax)
	{
		// Wrap to the left side
		Location.X = WrapMin;
		SetActorLocation(Location);
	}
	else if (!bMovesRight && Location.X < WrapMin)
	{
		// Wrap to the right side
		Location.X = WrapMax;
		SetActorLocation(Location);
	}
}

float AHazardBase::GetWorldWidth() const
{
	return static_cast<float>(HazardWidthCells) * GridCellSize;
}

float AHazardBase::GetWrapMinX() const
{
	return -GetWorldWidth();
}

float AHazardBase::GetWrapMaxX() const
{
	return static_cast<float>(GridColumns) * GridCellSize + GetWorldWidth();
}

void AHazardBase::SetupMeshForHazardType()
{
	// Hide the constructor's placeholder mesh — multi-part model replaces it
	if (MeshComponent)
	{
		MeshComponent->SetVisibility(false);
		MeshComponent->SetStaticMesh(nullptr);
	}

	// Car colors vary per lane. WebFrogger uses car1=red, car2=blue, car3=orange.
	// We assign based on movement direction as a simple differentiator.
	FLinearColor CarColor = bMovesRight
		? FLinearColor(0.933f, 0.188f, 0.188f)   // Red (car1)
		: FLinearColor(0.200f, 0.400f, 1.0f);     // Blue (car2)

	switch (HazardType)
	{
	case EHazardType::Car:
	case EHazardType::Motorcycle:
		FModelFactory::BuildCarModel(this, CarColor);
		break;
	case EHazardType::Truck:
		FModelFactory::BuildTruckModel(this);
		break;
	case EHazardType::Bus:
		FModelFactory::BuildBusModel(this);
		break;
	case EHazardType::SmallLog:
	case EHazardType::LargeLog:
		FModelFactory::BuildLogModel(this, HazardWidthCells);
		break;
	case EHazardType::TurtleGroup:
		FModelFactory::BuildTurtleGroupModel(this, HazardWidthCells);
		break;
	}
}
```

---

## File 3: LaneManager.cpp

**Lines 1-312** — Complete file

```cpp
// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/LaneManager.h"
#include "Core/HazardBase.h"
#include "Engine/World.h"

ALaneManager::ALaneManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ALaneManager::BeginPlay()
{
	Super::BeginPlay();

	if (LaneConfigs.Num() == 0)
	{
		SetupDefaultLaneConfigs();
	}

	for (const FLaneConfig& Config : LaneConfigs)
	{
		if (Config.LaneType == ELaneType::Road || Config.LaneType == ELaneType::River)
		{
			SpawnLaneHazards(Config);
		}
	}
}

ELaneType ALaneManager::GetLaneTypeAtRow(int32 Row) const
{
	for (const FLaneConfig& Config : LaneConfigs)
	{
		if (Config.RowIndex == Row)
		{
			return Config.LaneType;
		}
	}

	// Rows not in any config are safe by default
	return ELaneType::Safe;
}

bool ALaneManager::IsRowSafe(int32 Row) const
{
	ELaneType Type = GetLaneTypeAtRow(Row);
	return (Type == ELaneType::Safe || Type == ELaneType::Goal);
}

bool ALaneManager::ValidateGaps() const
{
	for (const FLaneConfig& Config : LaneConfigs)
	{
		if (Config.LaneType == ELaneType::Safe || Config.LaneType == ELaneType::Goal)
		{
			continue;
		}

		// A lane is impossible if a single hazard is wider than the entire grid
		// minus the minimum gap needed for the frog to pass (1 cell)
		if (Config.HazardWidth >= GridColumns)
		{
			return false;
		}

		// With multiple hazards evenly spaced, verify there is room for at least
		// one gap of MinGapCells between consecutive hazards.
		// Total space per hazard slot = HazardWidth + MinGapCells
		// Number of slots that fit = GridColumns / (HazardWidth + MinGapCells)
		// If zero slots fit, the config is invalid.
		int32 SlotSize = Config.HazardWidth + FMath::Max(Config.MinGapCells, 1);
		if (SlotSize > GridColumns)
		{
			return false;
		}
	}

	return true;
}

void ALaneManager::SetupDefaultLaneConfigs()
{
	LaneConfigs.Empty();

	// -- Road lanes (rows 1-5) --------------------------------------------

	// Row 1: Car, 150 UU/s, width 1, gap 3, LEFT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::Road;
		C.HazardType = EHazardType::Car;
		C.Speed = 150.0f;
		C.HazardWidth = 1;
		C.MinGapCells = 3;
		C.bMovesRight = false;
		C.RowIndex = 1;
		LaneConfigs.Add(C);
	}

	// Row 2: Truck, 100 UU/s, width 2, gap 4, RIGHT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::Road;
		C.HazardType = EHazardType::Truck;
		C.Speed = 100.0f;
		C.HazardWidth = 2;
		C.MinGapCells = 4;
		C.bMovesRight = true;
		C.RowIndex = 2;
		LaneConfigs.Add(C);
	}

	// Row 3: Car, 200 UU/s, width 1, gap 2, LEFT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::Road;
		C.HazardType = EHazardType::Car;
		C.Speed = 200.0f;
		C.HazardWidth = 1;
		C.MinGapCells = 2;
		C.bMovesRight = false;
		C.RowIndex = 3;
		LaneConfigs.Add(C);
	}

	// Row 4: Bus, 175 UU/s, width 2, gap 3, RIGHT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::Road;
		C.HazardType = EHazardType::Bus;
		C.Speed = 175.0f;
		C.HazardWidth = 2;
		C.MinGapCells = 3;
		C.bMovesRight = true;
		C.RowIndex = 4;
		LaneConfigs.Add(C);
	}

	// Row 5: Motorcycle, 250 UU/s, width 1, gap 2, LEFT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::Road;
		C.HazardType = EHazardType::Motorcycle;
		C.Speed = 250.0f;
		C.HazardWidth = 1;
		C.MinGapCells = 2;
		C.bMovesRight = false;
		C.RowIndex = 5;
		LaneConfigs.Add(C);
	}

	// -- River lanes (rows 7-12) ------------------------------------------

	// Row 7: SmallLog, 100 UU/s, width 2, gap 2, RIGHT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::River;
		C.HazardType = EHazardType::SmallLog;
		C.Speed = 100.0f;
		C.HazardWidth = 2;
		C.MinGapCells = 2;
		C.bMovesRight = true;
		C.RowIndex = 7;
		LaneConfigs.Add(C);
	}

	// Row 8: TurtleGroup, 80 UU/s, width 3, gap 2, LEFT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::River;
		C.HazardType = EHazardType::TurtleGroup;
		C.Speed = 80.0f;
		C.HazardWidth = 3;
		C.MinGapCells = 2;
		C.bMovesRight = false;
		C.RowIndex = 8;
		LaneConfigs.Add(C);
	}

	// Row 9: LargeLog, 120 UU/s, width 4, gap 2, RIGHT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::River;
		C.HazardType = EHazardType::LargeLog;
		C.Speed = 120.0f;
		C.HazardWidth = 4;
		C.MinGapCells = 2;
		C.bMovesRight = true;
		C.RowIndex = 9;
		LaneConfigs.Add(C);
	}

	// Row 10: SmallLog, 100 UU/s, width 2, gap 2, LEFT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::River;
		C.HazardType = EHazardType::SmallLog;
		C.Speed = 100.0f;
		C.HazardWidth = 2;
		C.MinGapCells = 2;
		C.bMovesRight = false;
		C.RowIndex = 10;
		LaneConfigs.Add(C);
	}

	// Row 11: TurtleGroup, 80 UU/s, width 3, gap 2, RIGHT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::River;
		C.HazardType = EHazardType::TurtleGroup;
		C.Speed = 80.0f;
		C.HazardWidth = 3;
		C.MinGapCells = 2;
		C.bMovesRight = true;
		C.RowIndex = 11;
		LaneConfigs.Add(C);
	}

	// Row 12: LargeLog, 150 UU/s, width 4, gap 2, LEFT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::River;
		C.HazardType = EHazardType::LargeLog;
		C.Speed = 150.0f;
		C.HazardWidth = 4;
		C.MinGapCells = 2;
		C.bMovesRight = false;
		C.RowIndex = 12;
		LaneConfigs.Add(C);
	}
}

void ALaneManager::SpawnLaneHazards(const FLaneConfig& Config)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<float> XPositions;
	CalculateSpawnPositions(Config, XPositions);

	TArray<AHazardBase*>& LaneHazards = HazardPool.FindOrAdd(Config.RowIndex);

	float YPos = static_cast<float>(Config.RowIndex) * GridCellSize;

	for (float XPos : XPositions)
	{
		FVector SpawnLocation(XPos, YPos, 0.0f);
		FRotator SpawnRotation = FRotator::ZeroRotator;

		AHazardBase* Hazard = World->SpawnActor<AHazardBase>(
			AHazardBase::StaticClass(),
			SpawnLocation,
			SpawnRotation
		);

		if (Hazard)
		{
			Hazard->InitFromConfig(Config, GridCellSize, GridColumns);
			LaneHazards.Add(Hazard);
		}
	}
}

void ALaneManager::ApplyWaveDifficulty(float SpeedMultiplier, int32 WaveNumber)
{
	for (auto& Pair : HazardPool)
	{
		for (AHazardBase* Hazard : Pair.Value)
		{
			if (!Hazard)
			{
				continue;
			}

			Hazard->Speed = Hazard->BaseSpeed * SpeedMultiplier;

			if (Hazard->HazardType == EHazardType::TurtleGroup)
			{
				Hazard->CurrentWave = WaveNumber;
			}
		}
	}
}

void ALaneManager::AddHazardToPool(int32 RowIndex, AHazardBase* Hazard)
{
	HazardPool.FindOrAdd(RowIndex).Add(Hazard);
}

void ALaneManager::CalculateSpawnPositions(const FLaneConfig& Config, TArray<float>& OutXPositions) const
{
	float GridWorldWidth = static_cast<float>(GridColumns) * GridCellSize;
	float HazardWorldWidth = static_cast<float>(Config.HazardWidth) * GridCellSize;
	float GapWorldWidth = static_cast<float>(FMath::Max(Config.MinGapCells, 1)) * GridCellSize;

	// Evenly space hazards across the lane width, accounting for wrapping
	float SlotWidth = HazardWorldWidth + GapWorldWidth;
	int32 NumHazards = FMath::Max(1, FMath::FloorToInt(GridWorldWidth / SlotWidth));
	NumHazards = FMath::Min(NumHazards, HazardsPerLane);

	float Spacing = GridWorldWidth / static_cast<float>(NumHazards);

	for (int32 i = 0; i < NumHazards; ++i)
	{
		float XPos = static_cast<float>(i) * Spacing;
		OutXPositions.Add(XPos);
	}
}
```

---

## File 4: HazardBase.h

**Lines 1-134** — Complete file

```cpp
// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/LaneTypes.h"
#include "HazardBase.generated.h"

class UBoxComponent;
class UStaticMesh;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ESubmergePhase : uint8
{
	Surface		UMETA(DisplayName = "Surface"),
	Warning		UMETA(DisplayName = "Warning"),
	Submerged	UMETA(DisplayName = "Submerged")
};

UCLASS()
class UNREALFROG_API AHazardBase : public AActor
{
	GENERATED_BODY()

public:
	AHazardBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// -- Tunable parameters -----------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	float Speed = 0.0f;

	/** Original speed from lane config. Used by ApplyWaveDifficulty to avoid compounding. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hazard")
	float BaseSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	int32 HazardWidthCells = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	bool bMovesRight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	EHazardType HazardType = EHazardType::Car;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	bool bIsRideable = false;

	// -- Turtle submerge --------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard|Turtle")
	float SurfaceDuration = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard|Turtle")
	float WarningDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard|Turtle")
	float SubmergeDuration = 2.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsSubmerged = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	ESubmergePhase SubmergePhase = ESubmergePhase::Surface;

	/** Current wave number — used to gate submerge (Wave 2+ only). Set by LaneManager. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard|Turtle")
	int32 CurrentWave = 1;

	// -- Grid references (set by LaneManager) -----------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridColumns = 13;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float GridCellSize = 100.0f;

	// -- Public interface -------------------------------------------------

	/** Moves the hazard along the X axis. Exposed for unit testing. */
	void TickMovement(float DeltaTime);

	/** Updates turtle submerge timer. Exposed for unit testing. */
	void TickSubmerge(float DeltaTime);

	/** Configure this hazard from a lane config. Called by LaneManager. */
	void InitFromConfig(const FLaneConfig& Config, float InGridCellSize, int32 InGridColumns);

protected:
	// -- Components -------------------------------------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

private:
	// -- Wrapping ---------------------------------------------------------

	/** Teleport hazard to the opposite side when it leaves the grid */
	void WrapPosition();

	/** Width of the hazard in world units */
	float GetWorldWidth() const;

	/** Left boundary for wrapping (includes buffer for hazard width) */
	float GetWrapMinX() const;

	/** Right boundary for wrapping (includes buffer for hazard width) */
	float GetWrapMaxX() const;

	// -- Mesh setup -------------------------------------------------------

	/** Configure mesh shape, scale, and color based on HazardType. Called in BeginPlay. */
	void SetupMeshForHazardType();

	/** Cached mesh assets loaded in constructor via ConstructorHelpers */
	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMeshAsset;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CylinderMeshAsset;

	// -- Turtle submerge state --------------------------------------------

	float SubmergeTimer = 0.0f;
};
```
