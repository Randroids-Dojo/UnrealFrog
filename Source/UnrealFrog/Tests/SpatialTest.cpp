// Copyright UnrealFrog Team. All Rights Reserved.
//
// [Spatial] Position assertion tests.
//
// These tests verify that the engine actually places spawned actors at the
// requested world position. Logic tests verify formulas and state machines;
// spatial tests verify the render-visible result of SpawnActor.
//
// Agreement Section 22 — every actor-spawning system must have at least one
// test that creates a UWorld, spawns the actor, and asserts GetActorLocation().
//
// Sprint 9, Task #3 — Engine Architect

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Core/FrogCharacter.h"
#include "Core/FroggerCameraActor.h"
#include "Core/FroggerVFXManager.h"
#include "Core/HazardBase.h"
#include "Core/LaneManager.h"
#include "Core/UnrealFrogGameMode.h"

#if WITH_AUTOMATION_TESTS

namespace SpatialTestHelper
{
	/** Create a minimal game world for spatial testing. Caller is responsible
	 *  for calling DestroyWorld when done. */
	UWorld* CreateTestWorld()
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
		check(World);
		World->InitializeActorsForPlay(FURL());
		return World;
	}

	void DestroyTestWorld(UWorld* World)
	{
		if (World)
		{
			// RouteEndPlay on all actors before destroying
			World->DestroyWorld(false);
			// GC will clean up the UWorld
		}
	}
}

// ---------------------------------------------------------------------------
// Test 1: VFX actor spawns at the requested world position
//
// The VFX system had a 7-sprint bug where all VFX spawned at world origin
// because AActor has no RootComponent by default, and SpawnActor silently
// discards the FTransform without one. The fix (Sprint 8 hotfix) sets
// RootComponent before RegisterComponent. This test catches regression.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSpatial_VFX_SpawnPosition,
	"UnrealFrog.Spatial.VFX.SpawnPosition",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpatial_VFX_SpawnPosition::RunTest(const FString& Parameters)
{
	UWorld* World = SpatialTestHelper::CreateTestWorld();

	// Spawn a plain AActor the same way VFXManager::SpawnVFXActor does:
	// create mesh component, set as root, register, then SetActorLocation.
	FVector DesiredLocation(600.0, 400.0, 0.0);

	AActor* VFXActor = World->SpawnActor<AActor>(AActor::StaticClass());
	TestNotNull(TEXT("VFX actor spawned"), VFXActor);

	if (VFXActor)
	{
		// Reproduce the VFXManager pattern: root component before location
		UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(VFXActor);
		VFXActor->SetRootComponent(MeshComp);
		MeshComp->RegisterComponent();
		VFXActor->SetActorLocation(DesiredLocation);

		FVector ActualLocation = VFXActor->GetActorLocation();
		double Tolerance = 1.0;
		TestNearlyEqual(TEXT("VFX X position"), ActualLocation.X, 600.0, Tolerance);
		TestNearlyEqual(TEXT("VFX Y position"), ActualLocation.Y, 400.0, Tolerance);
		TestNearlyEqual(TEXT("VFX Z position"), ActualLocation.Z, 0.0, Tolerance);
	}

	SpatialTestHelper::DestroyTestWorld(World);
	return true;
}

// ---------------------------------------------------------------------------
// Test 2: VFX actor WITHOUT RootComponent lands at origin (regression guard)
//
// This test proves the failure mode: without a RootComponent, SpawnActor
// with a later SetActorLocation is silently discarded. If this test ever
// starts passing with the correct location, the engine behavior changed.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSpatial_VFX_NoRootComponentFallsToOrigin,
	"UnrealFrog.Spatial.VFX.NoRootComponentFallsToOrigin",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpatial_VFX_NoRootComponentFallsToOrigin::RunTest(const FString& Parameters)
{
	UWorld* World = SpatialTestHelper::CreateTestWorld();

	FVector DesiredLocation(600.0, 400.0, 0.0);

	AActor* BadActor = World->SpawnActor<AActor>(AActor::StaticClass());
	TestNotNull(TEXT("Actor spawned"), BadActor);

	if (BadActor)
	{
		// Do NOT set RootComponent — this is the bug pattern
		UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(BadActor);
		MeshComp->RegisterComponent();
		BadActor->SetActorLocation(DesiredLocation);

		FVector ActualLocation = BadActor->GetActorLocation();
		// Without RootComponent, SetActorLocation is a no-op.
		// The actor stays at origin.
		double Tolerance = 1.0;
		TestNearlyEqual(TEXT("Bad actor X is at origin"), ActualLocation.X, 0.0, Tolerance);
		TestNearlyEqual(TEXT("Bad actor Y is at origin"), ActualLocation.Y, 0.0, Tolerance);
	}

	SpatialTestHelper::DestroyTestWorld(World);
	return true;
}

// ---------------------------------------------------------------------------
// Test 3: FrogCharacter spawns at expected grid start position
//
// Default GridPosition is (6, 0) = world position (600, 0, 0).
// FrogCharacter has a RootComponent (CapsuleComponent) set in constructor,
// so SpawnActor should respect the transform.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSpatial_Frog_SpawnPosition,
	"UnrealFrog.Spatial.Frog.SpawnPosition",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpatial_Frog_SpawnPosition::RunTest(const FString& Parameters)
{
	UWorld* World = SpatialTestHelper::CreateTestWorld();

	// Frog start: grid (6,0) = world (600, 0, 0)
	FVector ExpectedPos(600.0, 0.0, 0.0);
	FTransform SpawnTransform(FRotator::ZeroRotator, ExpectedPos);

	AFrogCharacter* Frog = World->SpawnActor<AFrogCharacter>(
		AFrogCharacter::StaticClass(), SpawnTransform);
	TestNotNull(TEXT("Frog spawned"), Frog);

	if (Frog)
	{
		FVector ActualLocation = Frog->GetActorLocation();
		double Tolerance = 1.0;
		TestNearlyEqual(TEXT("Frog X at grid column 6"), ActualLocation.X, 600.0, Tolerance);
		TestNearlyEqual(TEXT("Frog Y at grid row 0"), ActualLocation.Y, 0.0, Tolerance);
	}

	SpatialTestHelper::DestroyTestWorld(World);
	return true;
}

// ---------------------------------------------------------------------------
// Test 4: Camera actor spawns and its component is at expected position
//
// AFroggerCameraActor sets the CameraComponent as root in constructor with
// relative location (600, 700, 2200). After spawning at origin, the
// component should be at that relative offset = world (600, 700, 2200).
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSpatial_Camera_SpawnPosition,
	"UnrealFrog.Spatial.Camera.SpawnPosition",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpatial_Camera_SpawnPosition::RunTest(const FString& Parameters)
{
	UWorld* World = SpatialTestHelper::CreateTestWorld();

	AFroggerCameraActor* CamActor = World->SpawnActor<AFroggerCameraActor>(
		AFroggerCameraActor::StaticClass());
	TestNotNull(TEXT("Camera actor spawned"), CamActor);

	if (CamActor && CamActor->CameraComponent)
	{
		// The actor itself spawns at world origin.
		// The CameraComponent's relative location is (600, 700, 2200).
		// World location of the component = actor location + relative offset.
		FVector ComponentWorldPos = CamActor->CameraComponent->GetComponentLocation();
		double Tolerance = 1.0;
		TestNearlyEqual(TEXT("Camera world X"), ComponentWorldPos.X, 600.0, Tolerance);
		TestNearlyEqual(TEXT("Camera world Y"), ComponentWorldPos.Y, 700.0, Tolerance);
		TestNearlyEqual(TEXT("Camera world Z"), ComponentWorldPos.Z, 2200.0, Tolerance);
	}

	SpatialTestHelper::DestroyTestWorld(World);
	return true;
}

// ---------------------------------------------------------------------------
// Test 5: Hazard actor spawns at the requested lane Y position
//
// HazardBase has a RootComponent (BoxComponent) set in constructor.
// When LaneManager spawns hazards, it places them at Y = RowIndex * GridCellSize.
// This test spawns a hazard at a specific lane position and verifies.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSpatial_Hazard_SpawnPosition,
	"UnrealFrog.Spatial.Hazard.SpawnPosition",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpatial_Hazard_SpawnPosition::RunTest(const FString& Parameters)
{
	UWorld* World = SpatialTestHelper::CreateTestWorld();

	// Lane row 3, cell size 100 → Y = 300
	int32 RowIndex = 3;
	float GridCellSize = 100.0f;
	float ExpectedY = static_cast<float>(RowIndex) * GridCellSize;
	float XPos = 200.0f;

	FVector SpawnPos(XPos, ExpectedY, 0.0f);
	FTransform HazardTransform(FRotator::ZeroRotator, SpawnPos);
	AHazardBase* Hazard = World->SpawnActor<AHazardBase>(
		AHazardBase::StaticClass(), HazardTransform);
	TestNotNull(TEXT("Hazard spawned"), Hazard);

	if (Hazard)
	{
		FVector ActualLocation = Hazard->GetActorLocation();
		double Tolerance = 1.0;
		TestNearlyEqual(TEXT("Hazard X position"), ActualLocation.X, static_cast<double>(XPos), Tolerance);
		TestNearlyEqual(TEXT("Hazard Y at lane row 3"), ActualLocation.Y, static_cast<double>(ExpectedY), Tolerance);
		TestNearlyEqual(TEXT("Hazard Z at ground level"), ActualLocation.Z, 0.0, Tolerance);
	}

	SpatialTestHelper::DestroyTestWorld(World);
	return true;
}

// ---------------------------------------------------------------------------
// Test 6: Home slot world position calculation matches expected grid coords
//
// HomeSlotColumns = {1, 4, 6, 8, 11}, HomeSlotRow = 14, CellSize = 100
// Slot 0: column 1, row 14 → world (100, 1400, 50)
// Slot 2: column 6, row 14 → world (600, 1400, 50)
// Slot 4: column 11, row 14 → world (1100, 1400, 50)
//
// This tests the VFXManager::GetHomeSlotWorldLocation formula. Without a
// world context, it falls back to ZeroVector (tested in VFXTest.cpp).
// Here we verify the formula produces correct grid-to-world mapping by
// replicating the calculation and comparing.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSpatial_HomeSlot_Position,
	"UnrealFrog.Spatial.HomeSlot.Position",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpatial_HomeSlot_Position::RunTest(const FString& Parameters)
{
	// Verify the home slot position formula matches expectations
	// HomeSlotColumns = {1, 4, 6, 8, 11}, Row = 14, CellSize = 100
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	struct FExpectedSlot
	{
		int32 Index;
		double ExpectedX;
		double ExpectedY;
		double ExpectedZ;
	};

	TArray<FExpectedSlot> Expected;
	Expected.Add({0, 100.0, 1400.0, 50.0});   // Column 1
	Expected.Add({1, 400.0, 1400.0, 50.0});   // Column 4
	Expected.Add({2, 600.0, 1400.0, 50.0});   // Column 6
	Expected.Add({3, 800.0, 1400.0, 50.0});   // Column 8
	Expected.Add({4, 1100.0, 1400.0, 50.0});  // Column 11

	for (const FExpectedSlot& Slot : Expected)
	{
		// Replicate the formula from VFXManager::GetHomeSlotWorldLocation:
		// X = Column * CellSize, Y = HomeSlotRow * CellSize, Z = 50
		double CellSize = 100.0;
		int32 Column = GM->HomeSlotColumns[Slot.Index];
		double CalcX = static_cast<double>(Column) * CellSize;
		double CalcY = static_cast<double>(GM->HomeSlotRow) * CellSize;
		double Tolerance = 0.1;

		TestNearlyEqual(
			*FString::Printf(TEXT("Slot %d X"), Slot.Index),
			CalcX, Slot.ExpectedX, Tolerance);
		TestNearlyEqual(
			*FString::Printf(TEXT("Slot %d Y"), Slot.Index),
			CalcY, Slot.ExpectedY, Tolerance);
	}

	// Verify GameMode default values match our expectations
	TestEqual(TEXT("HomeSlotRow is 14"), GM->HomeSlotRow, 14);
	TestEqual(TEXT("5 home slot columns"), GM->HomeSlotColumns.Num(), 5);

	return true;
}

// ---------------------------------------------------------------------------
// Test 7: Multiple hazards spawned at different lane rows end up at correct Y
//
// LaneManager::SpawnLaneHazards places hazards at Y = RowIndex * GridCellSize.
// Rather than calling BeginPlay (which requires world begin-play routing),
// we spawn hazards manually at the calculated positions and verify each one.
// This tests the spatial contract that HazardBase (with its BoxComponent root)
// respects SpawnActor transforms at different row offsets.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSpatial_LaneManager_MultiRowPositions,
	"UnrealFrog.Spatial.LaneManager.MultiRowPositions",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpatial_LaneManager_MultiRowPositions::RunTest(const FString& Parameters)
{
	UWorld* World = SpatialTestHelper::CreateTestWorld();

	// Replicate LaneManager's spawn logic: Y = RowIndex * GridCellSize
	float GridCellSize = 100.0f;
	int32 TestRows[] = {1, 5, 8, 12};
	double Tolerance = 1.0;

	for (int32 Row : TestRows)
	{
		double ExpectedY = static_cast<double>(Row) * static_cast<double>(GridCellSize);
		FVector SpawnPos(0.0f, static_cast<float>(Row) * GridCellSize, 0.0f);
		FTransform SpawnTransform(FRotator::ZeroRotator, SpawnPos);

		AHazardBase* Hazard = World->SpawnActor<AHazardBase>(
			AHazardBase::StaticClass(), SpawnTransform);

		if (Hazard)
		{
			FVector Loc = Hazard->GetActorLocation();
			TestNearlyEqual(
				*FString::Printf(TEXT("Hazard at row %d: Y position"), Row),
				Loc.Y, ExpectedY, Tolerance);
		}
		else
		{
			AddError(FString::Printf(TEXT("Failed to spawn hazard at row %d"), Row));
		}
	}

	SpatialTestHelper::DestroyTestWorld(World);
	return true;
}

#endif // WITH_AUTOMATION_TESTS
