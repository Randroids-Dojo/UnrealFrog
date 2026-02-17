// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/GroundBuilder.h"
#include "Core/FlatColorMaterial.h"
#include "Core/ModelFactory.h"
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

		// Track safe zone materials for wave color temperature updates
		if (Row == 0 || Row == 6)
		{
			SafeZoneMaterials.Add(DynMat);
		}
	}
}

FLinearColor AGroundBuilder::GetWaveColor(int32 WaveNumber) const
{
	// Color temperature progression: cool -> warm -> hot
	// Wave 1-2: green/blue (cool)
	// Wave 3-4: yellow (warm)
	// Wave 5-6: orange
	// Wave 7+:  red (hot)

	if (WaveNumber <= 2)
	{
		// Cool: green/blue tint
		return FLinearColor(0.15f, 0.75f, 0.5f);
	}
	else if (WaveNumber <= 4)
	{
		// Warm: yellow tint - lerp from cool to warm
		float T = static_cast<float>(WaveNumber - 2) / 2.0f;
		FLinearColor Cool(0.15f, 0.75f, 0.5f);
		FLinearColor Warm(0.8f, 0.75f, 0.15f);
		return FLinearColor::LerpUsingHSV(Cool, Warm, T);
	}
	else if (WaveNumber <= 6)
	{
		// Orange: lerp from warm to orange
		float T = static_cast<float>(WaveNumber - 4) / 2.0f;
		FLinearColor Warm(0.8f, 0.75f, 0.15f);
		FLinearColor Orange(0.9f, 0.45f, 0.1f);
		return FLinearColor::LerpUsingHSV(Warm, Orange, T);
	}
	else
	{
		// Hot: red
		return FLinearColor(0.9f, 0.2f, 0.1f);
	}
}

void AGroundBuilder::UpdateWaveColor(int32 WaveNumber)
{
	FLinearColor WaveColor = GetWaveColor(WaveNumber);
	for (UMaterialInstanceDynamic* DynMat : SafeZoneMaterials)
	{
		if (DynMat)
		{
			DynMat->SetVectorParameterValue(TEXT("Color"), WaveColor);
		}
	}
}

void AGroundBuilder::SpawnHomeSlotIndicator(int32 Column)
{
	// Position anchor for this home slot's lily pad model
	float CenterX = static_cast<float>(Column) * GridCellSize + GridCellSize * 0.5f;
	float CenterY = static_cast<float>(HomeSlotRow) * GridCellSize + GridCellSize * 0.5f;

	// Create a scene component as a position anchor, then spawn lily pad
	// model components as children. The factory attaches to Owner->GetRootComponent(),
	// so we create an anchor at the slot position and reparent the parts.
	USceneComponent* Anchor = NewObject<USceneComponent>(this,
		*FString::Printf(TEXT("HomeSlotAnchor_%d"), Column));
	Anchor->SetupAttachment(GetRootComponent());
	Anchor->SetWorldLocation(FVector(CenterX, CenterY, 1.0f));
	Anchor->RegisterComponent();

	// Spawn lily pad parts: cylinder pad + sphere flower
	// We use the factory's mesh accessors and color palette directly
	// rather than calling BuildLilyPadModel, since we need world positioning.
	UStaticMesh* CylinderMesh = FModelFactory::GetCylinderMesh();
	UStaticMesh* SphereMesh = FModelFactory::GetSphereMesh();

	if (!CylinderMesh || !SphereMesh)
	{
		return;
	}

	// Pad: flattened cylinder (0.8 diameter, 0.05 height)
	UStaticMeshComponent* Pad = NewObject<UStaticMeshComponent>(this,
		*FString::Printf(TEXT("LilyPad_%d"), Column));
	Pad->SetStaticMesh(CylinderMesh);
	Pad->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Pad->SetupAttachment(Anchor);
	Pad->SetRelativeLocation(FVector(0, 0, 2));
	Pad->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.05f));
	Pad->RegisterComponent();

	if (UMaterial* FlatColor = GetOrCreateFlatColorMaterial())
	{
		Pad->SetMaterial(0, FlatColor);
	}
	UMaterialInstanceDynamic* PadMat = Pad->CreateAndSetMaterialInstanceDynamic(0);
	if (PadMat)
	{
		PadMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.067f, 0.467f, 0.067f));
	}

	// Flower: small sphere
	UStaticMeshComponent* Flower = NewObject<UStaticMeshComponent>(this,
		*FString::Printf(TEXT("LilyFlower_%d"), Column));
	Flower->SetStaticMesh(SphereMesh);
	Flower->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Flower->SetupAttachment(Anchor);
	Flower->SetRelativeLocation(FVector(15, -10, 8));
	Flower->SetRelativeScale3D(FVector(0.24f, 0.24f, 0.24f));
	Flower->RegisterComponent();

	if (UMaterial* FlatColor = GetOrCreateFlatColorMaterial())
	{
		Flower->SetMaterial(0, FlatColor);
	}
	UMaterialInstanceDynamic* FlowerMat = Flower->CreateAndSetMaterialInstanceDynamic(0);
	if (FlowerMat)
	{
		FlowerMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.412f, 0.706f));
	}
}
