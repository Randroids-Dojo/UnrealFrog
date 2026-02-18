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
	RowIndex = Config.RowIndex;

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

	// Car colors vary by row. WebFrogger uses car1=red, car2=blue, car3=orange.
	FLinearColor CarColor;
	switch (RowIndex)
	{
	case 1:  CarColor = FLinearColor(0.933f, 0.188f, 0.188f); break; // Red
	case 3:  CarColor = FLinearColor(1.0f, 0.667f, 0.0f);     break; // Orange
	default: CarColor = FLinearColor(0.200f, 0.400f, 1.0f);   break; // Blue (fallback)
	}

	switch (HazardType)
	{
	case EHazardType::Car:
		FModelFactory::BuildCarModel(this, CarColor);
		break;
	case EHazardType::RaceCar:
		FModelFactory::BuildRaceCarModel(this);
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
