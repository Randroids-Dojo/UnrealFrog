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
	const FLinearColor CarRed(0.933f, 0.188f, 0.188f);    // 0xff3333
	const FLinearColor CarBlue(0.200f, 0.400f, 1.0f);     // 0x3366ff
	const FLinearColor CarOrange(1.0f, 0.667f, 0.0f);     // 0xffaa00
	const FLinearColor Cabin(0.533f, 0.800f, 1.0f);
	const FLinearColor Wheel(0.133f, 0.133f, 0.133f);
	const FLinearColor Truck(0.533f, 0.267f, 0.533f);
	const FLinearColor Trailer(0.667f, 0.400f, 0.667f);
	const FLinearColor Bus(0.933f, 0.733f, 0.0f);
	const FLinearColor BusWindow(0.533f, 0.800f, 1.0f);
	const FLinearColor RaceCar(1.0f, 0.0f, 1.0f);         // 0xff00ff magenta

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
// Bus Model — 12 components (body + 7 windows + 4 wheels)
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

	// 7 windows matching WebFrogger: i from -3 to +3, spacing 0.2 (20 UU)
	// WebFrogger: BoxGeometry(0.15, 0.2, 0.62) at (i*0.2, 0.4, 0)
	// -> Cube scale (0.15, 0.62, 0.2), pos (I*20, 0, 40)
	for (int32 I = -3; I <= 3; ++I)
	{
		FString WinName = FString::Printf(TEXT("BusWindow_%d"), I + 3);
		AddMeshComponent(Owner, *WinName,
			Cube, FVector(I * 20, 0, 40), FVector(0.15, 0.62, 0.2), ModelColors::BusWindow);
	}

	// Wheels: WebFrogger sx in [-0.7, 0.7], sz in [-1, 1]
	int32 WheelIdx = 0;
	for (int32 SX = -1; SX <= 1; SX += 2)
	{
		for (int32 SZ = -1; SZ <= 1; SZ += 2)
		{
			FString WheelName = FString::Printf(TEXT("BusWheel_%d"), WheelIdx++);
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
// Log Model — 3 components (main cylinder + 2 end caps)
// =============================================================================
// WebFrogger reference: createLog(length) in index.html:439

UStaticMeshComponent* FModelFactory::BuildLogModel(AActor* Owner, int32 WidthCells)
{
	if (!Owner)
	{
		return nullptr;
	}

	UStaticMesh* Cylinder = GetCylinderMesh();

	// Main log cylinder laid along X axis (direction of travel).
	// Pitch 90 rotates the cylinder's Z-height toward X.
	float LogLength = static_cast<float>(WidthCells) * 0.9f;
	float LogWorldHalfLength = LogLength * 100.0f * 0.5f;

	UStaticMeshComponent* MainLog = AddMeshComponentRotated(Owner, TEXT("LogMain"),
		Cylinder,
		FVector(0, 0, 15),       // Raised so the log sits visibly above water
		FRotator(90, 0, 0),      // Pitch 90: cylinder height along X
		FVector(0.8, 0.8, LogLength),  // 80 UU diameter
		ModelColors::Log);

	// End caps: slightly larger radius (0.84 vs 0.8), short length, lighter brown
	// WebFrogger: CylinderGeometry(0.21, 0.21, 0.05) — radius 5% larger than main
	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		FString CapName = FString::Printf(TEXT("LogCap_%s"), Side < 0 ? TEXT("L") : TEXT("R"));
		AddMeshComponentRotated(Owner, *CapName,
			Cylinder,
			FVector(Side * LogWorldHalfLength, 0, 15),
			FRotator(90, 0, 0),
			FVector(0.84, 0.84, 0.05f),  // Slightly wider, very short
			ModelColors::LogCap);
	}

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

// =============================================================================
// Race Car Model — 7 components (body, spoiler, cockpit, 4 wheels)
// =============================================================================
// WebFrogger reference: createRaceCar() in index.html

UStaticMeshComponent* FModelFactory::BuildRaceCarModel(AActor* Owner)
{
	if (!Owner)
	{
		return nullptr;
	}

	UStaticMesh* Cube = GetCubeMesh();
	UStaticMesh* Sphere = GetSphereMesh();
	UStaticMesh* Cylinder = GetCylinderMesh();

	// Body: BoxGeometry(0.9, 0.2, 0.4) at y=0.15
	// -> Cube scale (0.9, 0.4, 0.2), pos (0, 0, 15)
	UStaticMeshComponent* Body = AddMeshComponent(Owner, TEXT("RaceCarBody"),
		Cube, FVector(0, 0, 15), FVector(0.9, 0.4, 0.2), ModelColors::RaceCar);

	// Spoiler: BoxGeometry(0.05, 0.15, 0.5) at (0.4, 0.3, 0)
	// -> Cube scale (0.05, 0.5, 0.15), pos (40, 0, 30)
	AddMeshComponent(Owner, TEXT("RaceCarSpoiler"),
		Cube, FVector(40, 0, 30), FVector(0.05, 0.5, 0.15), ModelColors::RaceCar);

	// Cockpit: SphereGeometry(0.15) at (-0.1, 0.3, 0), scale (1.5, 0.8, 1)
	// -> Sphere scale (0.15*1.5, 0.15*1, 0.15*0.8) = (0.225, 0.15, 0.12), pos (-10, 0, 30)
	AddMeshComponent(Owner, TEXT("RaceCarCockpit"),
		Sphere, FVector(-10, 0, 30), FVector(0.225, 0.15, 0.12), ModelColors::Cabin);

	// Wheels: CylinderGeometry(0.09, 0.09, 0.06) at (sx*0.35, 0.09, sz*0.24)
	// -> UE: (sx*35, -sz*24, 9), scale (0.18, 0.18, 0.06)
	int32 WheelIdx = 0;
	for (int32 SX = -1; SX <= 1; SX += 2)
	{
		for (int32 SZ = -1; SZ <= 1; SZ += 2)
		{
			FString WheelName = FString::Printf(TEXT("RaceCarWheel_%d"), WheelIdx++);
			AddMeshComponentRotated(Owner, *WheelName,
				Cylinder,
				FVector(SX * 35, -SZ * 24, 9),
				FRotator(0, 0, 90),
				FVector(0.18, 0.18, 0.06),
				ModelColors::Wheel);
		}
	}

	return Body;
}

// =============================================================================
// Small Frog Model — 3 components (miniature for filled home slots)
// =============================================================================
// WebFrogger reference: createSmallFrog() in index.html

UStaticMeshComponent* FModelFactory::BuildSmallFrogModel(AActor* Owner)
{
	if (!Owner)
	{
		return nullptr;
	}

	UStaticMesh* Cube = GetCubeMesh();
	UStaticMesh* Sphere = GetSphereMesh();

	// Body: BoxGeometry(0.3, 0.18, 0.25) at y=0.15
	// -> Cube scale (0.3, 0.25, 0.18), pos (0, 0, 15)
	UStaticMeshComponent* Body = AddMeshComponent(Owner, TEXT("SmallFrogBody"),
		Cube, FVector(0, 0, 15), FVector(0.3, 0.25, 0.18), ModelColors::FrogBody);

	// Eyes: SphereGeometry(0.05) at (side*0.1, 0.28, -0.08)
	// -> Sphere scale (0.1, 0.1, 0.1), pos (side*10, 8, 28)
	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		FString EyeName = FString::Printf(TEXT("SmallFrogEye_%s"), Side < 0 ? TEXT("L") : TEXT("R"));
		AddMeshComponent(Owner, *EyeName,
			Sphere, FVector(Side * 10, 8, 28), FVector(0.1, 0.1, 0.1), ModelColors::FrogEye);
	}

	return Body;
}
