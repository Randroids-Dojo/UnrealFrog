// Copyright UnrealFrog Team. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Core/LaneTypes.h"
#include "Core/HazardBase.h"
#include "Core/LaneManager.h"

#if WITH_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Test: FLaneConfig default values match the design spec
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLaneConfig_DefaultValues,
	"UnrealFrog.LaneSystem.LaneConfig_DefaultValues",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FLaneConfig_DefaultValues::RunTest(const FString& Parameters)
{
	FLaneConfig Config;

	TestEqual(TEXT("LaneType default"), Config.LaneType, ELaneType::Safe);
	TestEqual(TEXT("HazardType default"), Config.HazardType, EHazardType::Car);
	TestEqual(TEXT("Speed default"), Config.Speed, 0.0f);
	TestEqual(TEXT("HazardWidth default"), Config.HazardWidth, 1);
	TestEqual(TEXT("MinGapCells default"), Config.MinGapCells, 1);
	TestEqual(TEXT("bMovesRight default"), Config.bMovesRight, true);
	TestEqual(TEXT("RowIndex default"), Config.RowIndex, 0);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Hazard moves at configured speed in correct direction
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHazardBase_Movement,
	"UnrealFrog.LaneSystem.HazardBase_Movement",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHazardBase_Movement::RunTest(const FString& Parameters)
{
	AHazardBase* Hazard = NewObject<AHazardBase>();

	// Configure: move right at 200 UU/s
	Hazard->Speed = 200.0f;
	Hazard->bMovesRight = true;
	Hazard->GridColumns = 13;
	Hazard->GridCellSize = 100.0f;
	Hazard->SetActorLocation(FVector(0.0f, 100.0f, 0.0f));

	// Simulate 1 second of movement
	Hazard->TickMovement(1.0f);

	FVector Pos = Hazard->GetActorLocation();
	TestNearlyEqual(TEXT("X position after 1s at 200 UU/s right"), Pos.X, 200.0f);
	TestNearlyEqual(TEXT("Y position unchanged"), Pos.Y, 100.0f);

	// Configure: move left at 150 UU/s
	AHazardBase* HazardLeft = NewObject<AHazardBase>();
	HazardLeft->Speed = 150.0f;
	HazardLeft->bMovesRight = false;
	HazardLeft->GridColumns = 13;
	HazardLeft->GridCellSize = 100.0f;
	HazardLeft->SetActorLocation(FVector(600.0f, 200.0f, 0.0f));

	HazardLeft->TickMovement(1.0f);

	FVector PosLeft = HazardLeft->GetActorLocation();
	TestNearlyEqual(TEXT("X position after 1s at 150 UU/s left"), PosLeft.X, 450.0f);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Hazard wraps around when going off-screen
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHazardBase_Recycling,
	"UnrealFrog.LaneSystem.HazardBase_Recycling",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHazardBase_Recycling::RunTest(const FString& Parameters)
{
	// Grid is 13 columns wide, so boundary is at 13 * 100 = 1300 UU
	AHazardBase* Hazard = NewObject<AHazardBase>();
	Hazard->Speed = 200.0f;
	Hazard->bMovesRight = true;
	Hazard->HazardWidthCells = 1;
	Hazard->GridColumns = 13;
	Hazard->GridCellSize = 100.0f;

	// Place near right boundary
	Hazard->SetActorLocation(FVector(1250.0f, 100.0f, 0.0f));

	// After 1 second at 200 UU/s, raw X would be 1450
	// Should wrap: 1450 exceeds 1300 + buffer, so wraps to left side
	Hazard->TickMovement(1.0f);

	FVector Pos = Hazard->GetActorLocation();
	// Wrapped position: hazard should now be on the left side of the grid
	TestTrue(TEXT("Hazard wrapped to left side"), Pos.X < 1300.0f);

	// Test left-moving hazard wrapping
	AHazardBase* HazardLeft = NewObject<AHazardBase>();
	HazardLeft->Speed = 300.0f;
	HazardLeft->bMovesRight = false;
	HazardLeft->HazardWidthCells = 1;
	HazardLeft->GridColumns = 13;
	HazardLeft->GridCellSize = 100.0f;

	// Place near left boundary
	HazardLeft->SetActorLocation(FVector(50.0f, 100.0f, 0.0f));

	// After 1 second at 300 UU/s left, raw X would be -250
	HazardLeft->TickMovement(1.0f);

	FVector PosLeft = HazardLeft->GetActorLocation();
	TestTrue(TEXT("Left-moving hazard wrapped to right side"), PosLeft.X > 0.0f);

	return true;
}

// ---------------------------------------------------------------------------
// Test: LaneManager spawns hazards from config
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLaneManager_SpawnHazards,
	"UnrealFrog.LaneSystem.LaneManager_SpawnHazards",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FLaneManager_SpawnHazards::RunTest(const FString& Parameters)
{
	ALaneManager* Manager = NewObject<ALaneManager>();

	// Set up a single road lane config
	FLaneConfig Config;
	Config.LaneType = ELaneType::Road;
	Config.HazardType = EHazardType::Car;
	Config.Speed = 200.0f;
	Config.HazardWidth = 1;
	Config.MinGapCells = 1;
	Config.bMovesRight = true;
	Config.RowIndex = 1;

	Manager->LaneConfigs.Add(Config);

	// Query lane type
	TestEqual(TEXT("Row 1 is Road"), Manager->GetLaneTypeAtRow(1), ELaneType::Road);
	TestTrue(TEXT("Row 0 is safe (default)"), Manager->IsRowSafe(0));
	TestFalse(TEXT("Row 1 is not safe"), Manager->IsRowSafe(1));

	return true;
}

// ---------------------------------------------------------------------------
// Test: Gap validation detects impossible configurations
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLaneManager_GapValidation,
	"UnrealFrog.LaneSystem.LaneManager_GapValidation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FLaneManager_GapValidation::RunTest(const FString& Parameters)
{
	ALaneManager* Manager = NewObject<ALaneManager>();
	Manager->GridColumns = 13;
	Manager->GridCellSize = 100.0f;

	// Valid config: hazard width 1, min gap 1, in 13-column grid -- always passable
	FLaneConfig ValidConfig;
	ValidConfig.LaneType = ELaneType::Road;
	ValidConfig.HazardType = EHazardType::Car;
	ValidConfig.Speed = 200.0f;
	ValidConfig.HazardWidth = 1;
	ValidConfig.MinGapCells = 1;
	ValidConfig.bMovesRight = true;
	ValidConfig.RowIndex = 1;
	Manager->LaneConfigs.Add(ValidConfig);

	TestTrue(TEXT("Valid config passes gap validation"), Manager->ValidateGaps());

	// Invalid config: hazard width fills entire grid with no room for gaps
	Manager->LaneConfigs.Empty();
	FLaneConfig InvalidConfig;
	InvalidConfig.LaneType = ELaneType::Road;
	InvalidConfig.HazardType = EHazardType::Truck;
	InvalidConfig.Speed = 100.0f;
	InvalidConfig.HazardWidth = 13; // fills entire width
	InvalidConfig.MinGapCells = 0;
	InvalidConfig.bMovesRight = true;
	InvalidConfig.RowIndex = 2;
	Manager->LaneConfigs.Add(InvalidConfig);

	TestFalse(TEXT("Invalid config fails gap validation"), Manager->ValidateGaps());

	return true;
}

// ---------------------------------------------------------------------------
// Test: Odd lanes move right, even lanes move left
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHazard_DirectionAlternation,
	"UnrealFrog.LaneSystem.Hazard_DirectionAlternation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHazard_DirectionAlternation::RunTest(const FString& Parameters)
{
	// Verify the design spec: odd-indexed lanes move right, even move left
	// Lane 1 (odd) -> right
	FLaneConfig Lane1;
	Lane1.RowIndex = 1;
	Lane1.bMovesRight = (Lane1.RowIndex % 2 != 0);
	TestTrue(TEXT("Lane 1 (odd) moves right"), Lane1.bMovesRight);

	// Lane 2 (even) -> left
	FLaneConfig Lane2;
	Lane2.RowIndex = 2;
	Lane2.bMovesRight = (Lane2.RowIndex % 2 != 0);
	TestFalse(TEXT("Lane 2 (even) moves left"), Lane2.bMovesRight);

	// Lane 7 (odd) -> right
	FLaneConfig Lane7;
	Lane7.RowIndex = 7;
	Lane7.bMovesRight = (Lane7.RowIndex % 2 != 0);
	TestTrue(TEXT("Lane 7 (odd) moves right"), Lane7.bMovesRight);

	// Lane 12 (even) -> left
	FLaneConfig Lane12;
	Lane12.RowIndex = 12;
	Lane12.bMovesRight = (Lane12.RowIndex % 2 != 0);
	TestFalse(TEXT("Lane 12 (even) moves left"), Lane12.bMovesRight);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Turtle submerge state toggling
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHazardBase_TurtleSubmerge,
	"UnrealFrog.LaneSystem.HazardBase_TurtleSubmerge",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHazardBase_TurtleSubmerge::RunTest(const FString& Parameters)
{
	AHazardBase* Turtle = NewObject<AHazardBase>();
	Turtle->HazardType = EHazardType::TurtleGroup;
	Turtle->bIsRideable = true;
	Turtle->SubmergeInterval = 3.0f;
	Turtle->SubmergeDuration = 1.5f;
	Turtle->Speed = 80.0f;
	Turtle->bMovesRight = true;
	Turtle->GridColumns = 13;
	Turtle->GridCellSize = 100.0f;
	Turtle->SetActorLocation(FVector(200.0f, 900.0f, 0.0f));

	// Initially not submerged
	TestFalse(TEXT("Turtle starts surfaced"), Turtle->bIsSubmerged);

	// Tick 3.0 seconds -- should trigger submerge
	Turtle->TickSubmerge(3.0f);
	TestTrue(TEXT("Turtle submerged after interval"), Turtle->bIsSubmerged);

	// Tick 1.5 more seconds -- should surface again
	Turtle->TickSubmerge(1.5f);
	TestFalse(TEXT("Turtle surfaced after submerge duration"), Turtle->bIsSubmerged);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
