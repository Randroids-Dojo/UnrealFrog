// Copyright UnrealFrog Team. All Rights Reserved.
//
// [ModelFactory] Tests for multi-part model construction.
//
// Verifies that FModelFactory::Build* functions create the correct number
// of UStaticMeshComponents with collision disabled and valid meshes assigned.
//
// Sprint 12, Task #2 — Engine Architect

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Core/FrogCharacter.h"
#include "Core/HazardBase.h"
#include "Core/ModelFactory.h"

#if WITH_AUTOMATION_TESTS

namespace ModelFactoryTestHelper
{
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
			World->DestroyWorld(false);
		}
	}

	/** Count UStaticMeshComponents on an actor (excluding those from the constructor). */
	int32 CountMeshComponents(AActor* Actor)
	{
		TArray<UStaticMeshComponent*> Components;
		Actor->GetComponents<UStaticMeshComponent>(Components);
		return Components.Num();
	}

	/** Verify all UStaticMeshComponents have collision disabled. */
	bool AllMeshesHaveNoCollision(AActor* Actor, const TCHAR* SkipName)
	{
		TArray<UStaticMeshComponent*> Components;
		Actor->GetComponents<UStaticMeshComponent>(Components);
		for (UStaticMeshComponent* Comp : Components)
		{
			if (Comp->GetFName() == FName(SkipName))
			{
				continue;  // Skip the actor's original MeshComponent
			}
			if (Comp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
			{
				return false;
			}
		}
		return true;
	}
}

// ---------------------------------------------------------------------------
// Frog: BuildFrogModel creates 10 visual components
// (body, belly, 2 eyes, 2 pupils, 4 legs)
// Plus the 1 original MeshComponent from the constructor = 11 total
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FModelFactory_FrogComponentCount,
	"UnrealFrog.ModelFactory.FrogComponentCount",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FModelFactory_FrogComponentCount::RunTest(const FString& Parameters)
{
	UWorld* World = ModelFactoryTestHelper::CreateTestWorld();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AFrogCharacter* Frog = World->SpawnActor<AFrogCharacter>(
		AFrogCharacter::StaticClass(), SpawnParams);
	TestNotNull(TEXT("Frog spawned"), Frog);

	if (Frog)
	{
		// Before calling BuildFrogModel, the frog has 1 MeshComponent from the constructor
		int32 PreCount = ModelFactoryTestHelper::CountMeshComponents(Frog);

		UStaticMeshComponent* Body = FModelFactory::BuildFrogModel(Frog);
		TestNotNull(TEXT("BuildFrogModel returns body component"), Body);

		int32 PostCount = ModelFactoryTestHelper::CountMeshComponents(Frog);
		int32 AddedCount = PostCount - PreCount;

		// BuildFrogModel adds 10 components: body, belly, 2 eyes, 2 pupils, 4 legs
		TestEqual(TEXT("BuildFrogModel adds 10 mesh components"), AddedCount, 10);
	}

	ModelFactoryTestHelper::DestroyTestWorld(World);
	return true;
}

// ---------------------------------------------------------------------------
// Frog: All factory-created components have collision disabled
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FModelFactory_FrogNoCollision,
	"UnrealFrog.ModelFactory.FrogNoCollision",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FModelFactory_FrogNoCollision::RunTest(const FString& Parameters)
{
	UWorld* World = ModelFactoryTestHelper::CreateTestWorld();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AFrogCharacter* Frog = World->SpawnActor<AFrogCharacter>(
		AFrogCharacter::StaticClass(), SpawnParams);
	TestNotNull(TEXT("Frog spawned"), Frog);

	if (Frog)
	{
		FModelFactory::BuildFrogModel(Frog);
		TestTrue(TEXT("All factory components have NoCollision"),
			ModelFactoryTestHelper::AllMeshesHaveNoCollision(Frog, TEXT("MeshComponent")));
	}

	ModelFactoryTestHelper::DestroyTestWorld(World);
	return true;
}

// ---------------------------------------------------------------------------
// Car: BuildCarModel creates 6 visual components
// (body, cabin, 4 wheels)
// Plus 1 original MeshComponent from constructor = 7 total after
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FModelFactory_CarComponentCount,
	"UnrealFrog.ModelFactory.CarComponentCount",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FModelFactory_CarComponentCount::RunTest(const FString& Parameters)
{
	UWorld* World = ModelFactoryTestHelper::CreateTestWorld();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHazardBase* Hazard = World->SpawnActor<AHazardBase>(
		AHazardBase::StaticClass(), SpawnParams);
	TestNotNull(TEXT("Hazard spawned"), Hazard);

	if (Hazard)
	{
		int32 PreCount = ModelFactoryTestHelper::CountMeshComponents(Hazard);

		UStaticMeshComponent* Body = FModelFactory::BuildCarModel(
			Hazard, FLinearColor(0.933f, 0.188f, 0.188f));
		TestNotNull(TEXT("BuildCarModel returns body component"), Body);

		int32 PostCount = ModelFactoryTestHelper::CountMeshComponents(Hazard);
		int32 AddedCount = PostCount - PreCount;

		// BuildCarModel adds 6 components: body, cabin, 4 wheels
		// (Note: the original MeshComponent from HazardBase constructor is separate)
		TestEqual(TEXT("BuildCarModel adds 6 mesh components"), AddedCount, 6);
	}

	ModelFactoryTestHelper::DestroyTestWorld(World);
	return true;
}

// ---------------------------------------------------------------------------
// Truck: BuildTruckModel creates 8 visual components
// (cab, trailer, 6 wheels)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FModelFactory_TruckComponentCount,
	"UnrealFrog.ModelFactory.TruckComponentCount",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FModelFactory_TruckComponentCount::RunTest(const FString& Parameters)
{
	UWorld* World = ModelFactoryTestHelper::CreateTestWorld();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHazardBase* Hazard = World->SpawnActor<AHazardBase>(
		AHazardBase::StaticClass(), SpawnParams);
	TestNotNull(TEXT("Hazard spawned"), Hazard);

	if (Hazard)
	{
		int32 PreCount = ModelFactoryTestHelper::CountMeshComponents(Hazard);
		FModelFactory::BuildTruckModel(Hazard);
		int32 AddedCount = ModelFactoryTestHelper::CountMeshComponents(Hazard) - PreCount;

		// cab, trailer, 6 wheels = 8
		TestEqual(TEXT("BuildTruckModel adds 8 mesh components"), AddedCount, 8);
	}

	ModelFactoryTestHelper::DestroyTestWorld(World);
	return true;
}

// ---------------------------------------------------------------------------
// Bus: BuildBusModel creates 8 visual components
// (body, 3 windows, 4 wheels)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FModelFactory_BusComponentCount,
	"UnrealFrog.ModelFactory.BusComponentCount",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FModelFactory_BusComponentCount::RunTest(const FString& Parameters)
{
	UWorld* World = ModelFactoryTestHelper::CreateTestWorld();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHazardBase* Hazard = World->SpawnActor<AHazardBase>(
		AHazardBase::StaticClass(), SpawnParams);
	TestNotNull(TEXT("Hazard spawned"), Hazard);

	if (Hazard)
	{
		int32 PreCount = ModelFactoryTestHelper::CountMeshComponents(Hazard);
		FModelFactory::BuildBusModel(Hazard);
		int32 AddedCount = ModelFactoryTestHelper::CountMeshComponents(Hazard) - PreCount;

		// body, 3 windows, 4 wheels = 8
		TestEqual(TEXT("BuildBusModel adds 8 mesh components"), AddedCount, 8);
	}

	ModelFactoryTestHelper::DestroyTestWorld(World);
	return true;
}

// ---------------------------------------------------------------------------
// Log: BuildLogModel creates 1 visual component
// (single fat cylinder, no end caps — caps looked like car wheels from above)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FModelFactory_LogComponentCount,
	"UnrealFrog.ModelFactory.LogComponentCount",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FModelFactory_LogComponentCount::RunTest(const FString& Parameters)
{
	UWorld* World = ModelFactoryTestHelper::CreateTestWorld();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHazardBase* Hazard = World->SpawnActor<AHazardBase>(
		AHazardBase::StaticClass(), SpawnParams);
	TestNotNull(TEXT("Hazard spawned"), Hazard);

	if (Hazard)
	{
		int32 PreCount = ModelFactoryTestHelper::CountMeshComponents(Hazard);
		FModelFactory::BuildLogModel(Hazard, 3);  // 3-cell log
		int32 AddedCount = ModelFactoryTestHelper::CountMeshComponents(Hazard) - PreCount;

		// single fat cylinder, no end caps = 1
		TestEqual(TEXT("BuildLogModel adds 1 mesh component"), AddedCount, 1);
	}

	ModelFactoryTestHelper::DestroyTestWorld(World);
	return true;
}

// ---------------------------------------------------------------------------
// TurtleGroup: BuildTurtleGroupModel creates 3 * WidthCells components
// (shell, base, head per turtle)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FModelFactory_TurtleGroupComponentCount,
	"UnrealFrog.ModelFactory.TurtleGroupComponentCount",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FModelFactory_TurtleGroupComponentCount::RunTest(const FString& Parameters)
{
	UWorld* World = ModelFactoryTestHelper::CreateTestWorld();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHazardBase* Hazard = World->SpawnActor<AHazardBase>(
		AHazardBase::StaticClass(), SpawnParams);
	TestNotNull(TEXT("Hazard spawned"), Hazard);

	if (Hazard)
	{
		int32 PreCount = ModelFactoryTestHelper::CountMeshComponents(Hazard);
		int32 WidthCells = 3;
		FModelFactory::BuildTurtleGroupModel(Hazard, WidthCells);
		int32 AddedCount = ModelFactoryTestHelper::CountMeshComponents(Hazard) - PreCount;

		// 3 turtles * 3 components each = 9
		TestEqual(TEXT("BuildTurtleGroupModel adds 3*WidthCells mesh components"),
			AddedCount, WidthCells * 3);
	}

	ModelFactoryTestHelper::DestroyTestWorld(World);
	return true;
}

// ---------------------------------------------------------------------------
// LilyPad: BuildLilyPadModel creates 2 visual components
// (pad, flower)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FModelFactory_LilyPadComponentCount,
	"UnrealFrog.ModelFactory.LilyPadComponentCount",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FModelFactory_LilyPadComponentCount::RunTest(const FString& Parameters)
{
	UWorld* World = ModelFactoryTestHelper::CreateTestWorld();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHazardBase* Hazard = World->SpawnActor<AHazardBase>(
		AHazardBase::StaticClass(), SpawnParams);
	TestNotNull(TEXT("Hazard spawned"), Hazard);

	if (Hazard)
	{
		int32 PreCount = ModelFactoryTestHelper::CountMeshComponents(Hazard);
		FModelFactory::BuildLilyPadModel(Hazard);
		int32 AddedCount = ModelFactoryTestHelper::CountMeshComponents(Hazard) - PreCount;

		// pad, flower = 2
		TestEqual(TEXT("BuildLilyPadModel adds 2 mesh components"), AddedCount, 2);
	}

	ModelFactoryTestHelper::DestroyTestWorld(World);
	return true;
}

// ---------------------------------------------------------------------------
// Null safety: all Build* functions return nullptr when given nullptr
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FModelFactory_NullSafety,
	"UnrealFrog.ModelFactory.NullSafety",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FModelFactory_NullSafety::RunTest(const FString& Parameters)
{
	TestNull(TEXT("BuildFrogModel(nullptr)"), FModelFactory::BuildFrogModel(nullptr));
	TestNull(TEXT("BuildCarModel(nullptr)"), FModelFactory::BuildCarModel(nullptr, FLinearColor::Red));
	TestNull(TEXT("BuildTruckModel(nullptr)"), FModelFactory::BuildTruckModel(nullptr));
	TestNull(TEXT("BuildBusModel(nullptr)"), FModelFactory::BuildBusModel(nullptr));
	TestNull(TEXT("BuildLogModel(nullptr)"), FModelFactory::BuildLogModel(nullptr, 3));
	TestNull(TEXT("BuildTurtleGroupModel(nullptr)"), FModelFactory::BuildTurtleGroupModel(nullptr, 3));
	TestNull(TEXT("BuildLilyPadModel(nullptr)"), FModelFactory::BuildLilyPadModel(nullptr));

	return true;
}

// ---------------------------------------------------------------------------
// Mesh accessors: all three return valid meshes
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FModelFactory_MeshAccessors,
	"UnrealFrog.ModelFactory.MeshAccessors",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FModelFactory_MeshAccessors::RunTest(const FString& Parameters)
{
	TestNotNull(TEXT("GetCubeMesh returns valid mesh"), FModelFactory::GetCubeMesh());
	TestNotNull(TEXT("GetSphereMesh returns valid mesh"), FModelFactory::GetSphereMesh());
	TestNotNull(TEXT("GetCylinderMesh returns valid mesh"), FModelFactory::GetCylinderMesh());

	return true;
}

#endif // WITH_AUTOMATION_TESTS
