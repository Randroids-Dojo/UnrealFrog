// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/GroundBuilder.h"
#include "Core/FlatColorMaterial.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AGroundBuilder::AGroundBuilder()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Load the engine cube mesh -- used for all ground planes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubeFinder.Succeeded())
	{
		CubeMesh = CubeFinder.Object;
	}

	// Default home slot columns per game design spec
	HomeSlotColumns = {1, 4, 6, 8, 11};

	// Populate row definitions with default colors
	SetupDefaultRowDefinitions();
}

void AGroundBuilder::SetupDefaultRowDefinitions()
{
	RowDefinitions.Empty();
	RowDefinitions.Reserve(15);

	// Row 0: Start (safe) -- bright green
	RowDefinitions.Add({0, FLinearColor(0.2f, 0.8f, 0.2f)});

	// Rows 1-5: Road -- dark gray
	for (int32 Row = 1; Row <= 5; ++Row)
	{
		RowDefinitions.Add({Row, FLinearColor(0.3f, 0.3f, 0.3f)});
	}

	// Row 6: Median (safe) -- bright green
	RowDefinitions.Add({6, FLinearColor(0.2f, 0.8f, 0.2f)});

	// Rows 7-12: River -- blue
	for (int32 Row = 7; Row <= 12; ++Row)
	{
		RowDefinitions.Add({Row, FLinearColor(0.1f, 0.3f, 0.8f)});
	}

	// Row 13: Goal (lower) -- amber
	RowDefinitions.Add({13, FLinearColor(0.8f, 0.6f, 0.1f)});

	// Row 14: Goal (upper) -- dark green
	RowDefinitions.Add({14, FLinearColor(0.1f, 0.5f, 0.1f)});
}

void AGroundBuilder::BeginPlay()
{
	Super::BeginPlay();

	// Spawn one row plane for each definition
	for (const FGroundRowInfo& Info : RowDefinitions)
	{
		SpawnRowPlane(Info.Row, Info.Color);
	}

	// Spawn home slot indicators on the goal row
	for (int32 Col : HomeSlotColumns)
	{
		SpawnHomeSlotIndicator(Col);
	}
}

void AGroundBuilder::SpawnRowPlane(int32 Row, const FLinearColor& Color)
{
	if (!CubeMesh)
	{
		return;
	}

	UStaticMeshComponent* Plane = NewObject<UStaticMeshComponent>(this);
	if (!Plane)
	{
		return;
	}

	Plane->SetStaticMesh(CubeMesh);

	// Position: center of the row
	// X = half the grid width (13 columns * 100 UU / 2 = 650)
	// Y = row center (Row * CellSize + CellSize/2)
	// Z = -5 (half of 10 UU thickness, so top surface is at Z=0)
	float GridWidth = static_cast<float>(GridColumns) * GridCellSize;
	float CenterX = GridWidth * 0.5f;
	float CenterY = static_cast<float>(Row) * GridCellSize + GridCellSize * 0.5f;
	float CenterZ = -5.0f;

	Plane->SetWorldLocation(FVector(CenterX, CenterY, CenterZ));

	// Scale: 1300 x 100 x 10 UU
	// Engine cube is 100x100x100 UU, so scale factors are width/100
	float ScaleX = static_cast<float>(GridColumns);   // 13.0
	float ScaleY = 1.0f;                               // 100 UU = 1 cell
	float ScaleZ = 0.1f;                               // 10 UU thick
	Plane->SetWorldScale3D(FVector(ScaleX, ScaleY, ScaleZ));

	Plane->RegisterComponent();
	Plane->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);

	// Apply flat color material and set zone color
	if (UMaterial* FlatColor = GetOrCreateFlatColorMaterial())
	{
		Plane->SetMaterial(0, FlatColor);
	}
	UMaterialInstanceDynamic* DynMat = Plane->CreateAndSetMaterialInstanceDynamic(0);
	if (DynMat)
	{
		DynMat->SetVectorParameterValue(TEXT("Color"), Color);
	}
}

void AGroundBuilder::SpawnHomeSlotIndicator(int32 Column)
{
	if (!CubeMesh)
	{
		return;
	}

	UStaticMeshComponent* Indicator = NewObject<UStaticMeshComponent>(this);
	if (!Indicator)
	{
		return;
	}

	Indicator->SetStaticMesh(CubeMesh);

	// Position: center of the cell at (Column, HomeSlotRow)
	float CenterX = static_cast<float>(Column) * GridCellSize + GridCellSize * 0.5f;
	float CenterY = static_cast<float>(HomeSlotRow) * GridCellSize + GridCellSize * 0.5f;
	float CenterZ = 1.0f;  // Slightly elevated above the row plane

	Indicator->SetWorldLocation(FVector(CenterX, CenterY, CenterZ));

	// Scale: 80x80 UU plane (0.8 of a cell), thin
	Indicator->SetWorldScale3D(FVector(0.8f, 0.8f, 0.05f));

	Indicator->RegisterComponent();
	Indicator->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);

	// Apply flat color material and set home slot color
	if (UMaterial* FlatColor = GetOrCreateFlatColorMaterial())
	{
		Indicator->SetMaterial(0, FlatColor);
	}
	UMaterialInstanceDynamic* DynMat = Indicator->CreateAndSetMaterialInstanceDynamic(0);
	if (DynMat)
	{
		DynMat->SetVectorParameterValue(TEXT("Color"), HomeSlotColor);
	}
}
