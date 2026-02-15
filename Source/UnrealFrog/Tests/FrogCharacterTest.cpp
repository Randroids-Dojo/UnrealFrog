// Copyright UnrealFrog Team. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Core/FrogCharacter.h"

#if WITH_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Test: Default property values match the design spec
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFrogCharacter_DefaultValues,
	"UnrealFrog.Character.DefaultValues",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFrogCharacter_DefaultValues::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();

	TestEqual(TEXT("HopDuration default"), Frog->HopDuration, 0.15f);
	TestEqual(TEXT("HopArcHeight default"), Frog->HopArcHeight, 30.0f);
	TestEqual(TEXT("GridCellSize default"), Frog->GridCellSize, 100.0f);
	TestEqual(TEXT("InputBufferWindow default"), Frog->InputBufferWindow, 0.08f);
	TestEqual(TEXT("BackwardHopMultiplier default"), Frog->BackwardHopMultiplier, 1.0f);
	TestEqual(TEXT("GridColumns default"), Frog->GridColumns, 13);
	TestEqual(TEXT("GridRows default"), Frog->GridRows, 15);
	TestEqual(TEXT("bIsHopping default"), Frog->bIsHopping, false);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Grid position tracking and coordinate conversion
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFrogCharacter_GridPosition,
	"UnrealFrog.Character.GridPosition",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFrogCharacter_GridPosition::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();

	// Default grid position should be center-bottom of the grid
	TestEqual(TEXT("Default GridPosition.X"), Frog->GridPosition.X, 6);
	TestEqual(TEXT("Default GridPosition.Y"), Frog->GridPosition.Y, 0);

	// Test GridToWorld conversion
	FVector WorldPos = Frog->GridToWorld(FIntPoint(0, 0));
	TestEqual(TEXT("Grid(0,0) world X"), WorldPos.X, 0.0);
	TestEqual(TEXT("Grid(0,0) world Y"), WorldPos.Y, 0.0);

	FVector WorldPos2 = Frog->GridToWorld(FIntPoint(3, 5));
	TestEqual(TEXT("Grid(3,5) world X"), WorldPos2.X, 300.0);
	TestEqual(TEXT("Grid(3,5) world Y"), WorldPos2.Y, 500.0);

	// Test WorldToGrid conversion (round-trip)
	FIntPoint GridPos = Frog->WorldToGrid(FVector(300.0f, 500.0f, 0.0f));
	TestEqual(TEXT("World(300,500) -> Grid X"), GridPos.X, 3);
	TestEqual(TEXT("World(300,500) -> Grid Y"), GridPos.Y, 5);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Hop request initiates a hop when not already hopping
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFrogCharacter_HopRequest,
	"UnrealFrog.Character.HopRequest",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFrogCharacter_HopRequest::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();

	// Frog starts not hopping
	TestFalse(TEXT("Not hopping initially"), Frog->bIsHopping);

	// Request a forward hop (positive Y = forward)
	Frog->RequestHop(FVector(0.0f, 1.0f, 0.0f));

	TestTrue(TEXT("Hopping after RequestHop"), Frog->bIsHopping);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Input buffering is rejected early in a hop (outside InputBufferWindow)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFrogCharacter_InputBufferRejectsEarly,
	"UnrealFrog.Character.InputBufferRejectsEarly",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFrogCharacter_InputBufferRejectsEarly::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();

	// HopDuration=0.15, InputBufferWindow=0.1 → buffer starts at 0.05s into hop
	// At HopElapsed=0, TimeRemaining=0.15 > InputBufferWindow=0.1 → reject
	Frog->RequestHop(FVector(0.0f, 1.0f, 0.0f));
	TestTrue(TEXT("First hop started"), Frog->bIsHopping);

	// Request another hop immediately (HopElapsed=0, outside buffer window)
	Frog->RequestHop(FVector(1.0f, 0.0f, 0.0f));

	TestTrue(TEXT("Still hopping"), Frog->bIsHopping);
	TestFalse(TEXT("Input NOT buffered (too early)"), Frog->HasBufferedInput());

	return true;
}

// ---------------------------------------------------------------------------
// Test: Input buffering accepted when inside the InputBufferWindow
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFrogCharacter_InputBufferAcceptsInWindow,
	"UnrealFrog.Character.InputBufferAcceptsInWindow",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFrogCharacter_InputBufferAcceptsInWindow::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();

	// Set InputBufferWindow to cover the entire hop duration so any point
	// during the hop is inside the buffer window
	Frog->InputBufferWindow = Frog->HopDuration;

	Frog->RequestHop(FVector(0.0f, 1.0f, 0.0f));
	TestTrue(TEXT("First hop started"), Frog->bIsHopping);

	// At HopElapsed=0, TimeRemaining=0.15 <= InputBufferWindow=0.15 → accept
	Frog->RequestHop(FVector(1.0f, 0.0f, 0.0f));

	TestTrue(TEXT("Still hopping"), Frog->bIsHopping);
	TestTrue(TEXT("Input buffered (inside window)"), Frog->HasBufferedInput());

	return true;
}

// ---------------------------------------------------------------------------
// Test: All directions use uniform hop duration (multiplier is 1.0)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFrogCharacter_UniformHopDuration,
	"UnrealFrog.Character.UniformHopDuration",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFrogCharacter_UniformHopDuration::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();

	float ForwardDuration = Frog->GetHopDurationForDirection(FVector(0.0f, 1.0f, 0.0f));
	float BackwardDuration = Frog->GetHopDurationForDirection(FVector(0.0f, -1.0f, 0.0f));
	float SideDuration = Frog->GetHopDurationForDirection(FVector(1.0f, 0.0f, 0.0f));

	TestEqual(TEXT("Forward hop duration"), ForwardDuration, 0.15f);
	TestEqual(TEXT("Backward hop duration"), BackwardDuration, 0.15f);
	TestEqual(TEXT("Side hop duration"), SideDuration, 0.15f);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Frog cannot hop outside grid boundaries
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFrogCharacter_GridBounds,
	"UnrealFrog.Character.GridBounds",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFrogCharacter_GridBounds::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();

	// Valid positions
	TestTrue(TEXT("(0,0) is valid"), Frog->IsValidGridPosition(FIntPoint(0, 0)));
	TestTrue(TEXT("(12,14) is valid"), Frog->IsValidGridPosition(FIntPoint(12, 14)));
	TestTrue(TEXT("(6,7) is valid"), Frog->IsValidGridPosition(FIntPoint(6, 7)));

	// Invalid positions (out of bounds)
	TestFalse(TEXT("(-1,0) is invalid"), Frog->IsValidGridPosition(FIntPoint(-1, 0)));
	TestFalse(TEXT("(13,0) is invalid"), Frog->IsValidGridPosition(FIntPoint(13, 0)));
	TestFalse(TEXT("(0,-1) is invalid"), Frog->IsValidGridPosition(FIntPoint(0, -1)));
	TestFalse(TEXT("(0,15) is invalid"), Frog->IsValidGridPosition(FIntPoint(0, 15)));

	// Frog at left edge should not be able to hop left
	Frog->GridPosition = FIntPoint(0, 7);
	Frog->RequestHop(FVector(-1.0f, 0.0f, 0.0f));
	TestFalse(TEXT("Cannot hop left from column 0"), Frog->bIsHopping);

	// Frog at bottom edge should not be able to hop backward
	Frog->GridPosition = FIntPoint(6, 0);
	Frog->RequestHop(FVector(0.0f, -1.0f, 0.0f));
	TestFalse(TEXT("Cannot hop backward from row 0"), Frog->bIsHopping);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
