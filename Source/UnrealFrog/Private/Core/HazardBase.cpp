// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/HazardBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

AHazardBase::AHazardBase()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
	SetRootComponent(CollisionBox);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionBox);
}

void AHazardBase::BeginPlay()
{
	Super::BeginPlay();
}

void AHazardBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	SubmergeTimer += DeltaTime;

	if (!bIsSubmerged)
	{
		// Surfaced: wait for SubmergeInterval to elapse, then submerge
		if (SubmergeTimer >= SubmergeInterval)
		{
			bIsSubmerged = true;
			SubmergeTimer = 0.0f;
		}
	}
	else
	{
		// Submerged: wait for SubmergeDuration to elapse, then surface
		if (SubmergeTimer >= SubmergeDuration)
		{
			bIsSubmerged = false;
			SubmergeTimer = 0.0f;
		}
	}
}

void AHazardBase::InitFromConfig(const FLaneConfig& Config, float InGridCellSize, int32 InGridColumns)
{
	Speed = Config.Speed;
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
