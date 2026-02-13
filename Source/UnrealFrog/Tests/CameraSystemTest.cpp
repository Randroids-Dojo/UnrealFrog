// Copyright UnrealFrog Team. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Core/FroggerCameraActor.h"
#include "Camera/CameraComponent.h"

#if WITH_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Test: Camera component exists after construction
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraTest_ComponentExists,
	"UnrealFrog.Camera.ComponentExists",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraTest_ComponentExists::RunTest(const FString& Parameters)
{
	AFroggerCameraActor* CamActor = NewObject<AFroggerCameraActor>();

	TestNotNull(TEXT("CameraComponent should be non-null"), CamActor->CameraComponent.Get());

	return true;
}

// ---------------------------------------------------------------------------
// Test: Camera pitch is approximately -72 degrees
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraTest_Pitch,
	"UnrealFrog.Camera.Pitch",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraTest_Pitch::RunTest(const FString& Parameters)
{
	AFroggerCameraActor* CamActor = NewObject<AFroggerCameraActor>();

	FRotator Rotation = CamActor->CameraComponent->GetRelativeRotation();
	TestNearlyEqual(TEXT("Camera pitch should be -90 degrees (top-down)"), Rotation.Pitch, -90.0);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Camera FOV is 60 degrees
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraTest_FOV,
	"UnrealFrog.Camera.FOV",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraTest_FOV::RunTest(const FString& Parameters)
{
	AFroggerCameraActor* CamActor = NewObject<AFroggerCameraActor>();

	TestNearlyEqual(TEXT("Camera FOV should be 50 degrees"), CamActor->CameraComponent->FieldOfView, 50.0f);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Camera world position is at expected grid-center coordinates
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraTest_Position,
	"UnrealFrog.Camera.Position",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraTest_Position::RunTest(const FString& Parameters)
{
	AFroggerCameraActor* CamActor = NewObject<AFroggerCameraActor>();

	FVector CameraPos = CamActor->CameraComponent->GetRelativeLocation();
	TestNearlyEqual(TEXT("Camera X centered over grid"), CameraPos.X, 600.0, 1.0);
	TestNearlyEqual(TEXT("Camera Y centered over grid"), CameraPos.Y, 700.0, 1.0);
	TestNearlyEqual(TEXT("Camera Z high above grid"), CameraPos.Z, 2200.0, 1.0);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
