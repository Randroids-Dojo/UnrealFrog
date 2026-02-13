// Copyright UnrealFrog Team. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Core/FrogCharacter.h"
#include "Core/HazardBase.h"

#if WITH_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Test: FrogCharacter has a valid MeshComponent with a static mesh assigned
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMesh_FrogCharacterHasMesh,
	"UnrealFrog.Mesh.FrogCharacterHasMesh",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMesh_FrogCharacterHasMesh::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();

	TestNotNull(TEXT("MeshComponent should exist"), Frog->FindComponentByClass<UStaticMeshComponent>());

	UStaticMeshComponent* Mesh = Frog->FindComponentByClass<UStaticMeshComponent>();
	if (Mesh)
	{
		TestNotNull(TEXT("StaticMesh should be assigned"), Mesh->GetStaticMesh().Get());
	}

	return true;
}

// ---------------------------------------------------------------------------
// Test: HazardBase has a valid MeshComponent after construction
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMesh_HazardBaseHasMesh,
	"UnrealFrog.Mesh.HazardBaseHasMesh",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMesh_HazardBaseHasMesh::RunTest(const FString& Parameters)
{
	AHazardBase* Hazard = NewObject<AHazardBase>();

	TestNotNull(TEXT("MeshComponent should exist"), Hazard->FindComponentByClass<UStaticMeshComponent>());

	return true;
}

#endif // WITH_AUTOMATION_TESTS
