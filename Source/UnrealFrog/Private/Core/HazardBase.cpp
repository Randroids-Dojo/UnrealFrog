// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/HazardBase.h"
#include "Core/FlatColorMaterial.h"
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

	SetupMeshForHazardType();
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

void AHazardBase::SetupMeshForHazardType()
{
	if (!MeshComponent)
	{
		return;
	}

	// Determine if this is a river object (cylinder) or road object (cube)
	bool bIsRiverObject = (HazardType == EHazardType::SmallLog
		|| HazardType == EHazardType::LargeLog
		|| HazardType == EHazardType::TurtleGroup);

	// Assign the appropriate mesh
	if (bIsRiverObject && CylinderMeshAsset)
	{
		MeshComponent->SetStaticMesh(CylinderMeshAsset);

		// Rotate cylinder 90 degrees to lie flat along X axis
		MeshComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	}
	else if (!bIsRiverObject && CubeMeshAsset)
	{
		MeshComponent->SetStaticMesh(CubeMeshAsset);
	}

	// Scale based on HazardWidthCells
	// Default cube/cylinder is 100 UU (1 cell) in each axis
	// X: scale by width in cells
	// Y: 1.0 (one cell deep)
	// Z: 0.5 (half cell height so hazards don't look too tall)
	float ScaleX = static_cast<float>(HazardWidthCells);
	MeshComponent->SetWorldScale3D(FVector(ScaleX, 1.0f, 0.5f));

	// Select color based on hazard type
	FLinearColor HazardColor = FLinearColor::White;
	switch (HazardType)
	{
	case EHazardType::Car:
		HazardColor = FLinearColor(0.9f, 0.1f, 0.1f);  // Red
		break;
	case EHazardType::Truck:
		HazardColor = FLinearColor(0.6f, 0.1f, 0.1f);  // Dark red
		break;
	case EHazardType::Bus:
		HazardColor = FLinearColor(0.9f, 0.5f, 0.1f);  // Orange
		break;
	case EHazardType::Motorcycle:
		HazardColor = FLinearColor(0.9f, 0.9f, 0.1f);  // Yellow
		break;
	case EHazardType::SmallLog:
	case EHazardType::LargeLog:
		HazardColor = FLinearColor(0.5f, 0.3f, 0.1f);  // Brown
		break;
	case EHazardType::TurtleGroup:
		HazardColor = FLinearColor(0.1f, 0.5f, 0.2f);  // Dark green
		break;
	}

	// Apply flat color material and set color
	if (UMaterial* FlatColor = GetOrCreateFlatColorMaterial())
	{
		MeshComponent->SetMaterial(0, FlatColor);
	}
	UMaterialInstanceDynamic* DynMat = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	if (DynMat)
	{
		DynMat->SetVectorParameterValue(TEXT("Color"), HazardColor);
	}
}
