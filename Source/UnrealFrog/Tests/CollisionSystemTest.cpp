// Copyright UnrealFrog Team. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Core/FrogCharacter.h"
#include "Core/HazardBase.h"
#include "Core/LaneTypes.h"

#if WITH_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Test: Die() sets dead state and records the death type
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCollisionSystem_RoadHazardKillsFrog,
	"UnrealFrog.Collision.RoadHazardKillsFrog",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCollisionSystem_RoadHazardKillsFrog::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();

	// Frog is alive by default
	TestFalse(TEXT("Frog starts alive"), Frog->bIsDead);

	// Simulate road hazard killing the frog
	Frog->Die(EDeathType::Squish);

	TestTrue(TEXT("Frog is dead after Die(Squish)"), Frog->bIsDead);
	TestEqual(TEXT("Last death type is Squish"), Frog->LastDeathType, EDeathType::Squish);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Frog on river row without platform triggers Splash check
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCollisionSystem_RiverWithoutPlatform,
	"UnrealFrog.Collision.RiverWithoutPlatform",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCollisionSystem_RiverWithoutPlatform::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();

	// Place frog on a river row (row 7) with no platform
	Frog->GridPosition = FIntPoint(6, 7);
	Frog->CurrentPlatform = nullptr;

	// IsOnRiverRow should detect river rows (7-12)
	TestTrue(TEXT("Row 7 is a river row"), Frog->IsOnRiverRow());

	// CheckRiverDeath should return true when no platform is present
	TestTrue(TEXT("Frog should die in river without platform"), Frog->CheckRiverDeath());

	return true;
}

// ---------------------------------------------------------------------------
// Test: Frog riding a platform moves with its velocity
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCollisionSystem_RidingPlatform,
	"UnrealFrog.Collision.RidingPlatform",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCollisionSystem_RidingPlatform::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	AHazardBase* Log = NewObject<AHazardBase>();

	// Configure log moving right at 100 UU/s
	Log->Speed = 100.0f;
	Log->bMovesRight = true;
	Log->bIsRideable = true;
	Log->bIsSubmerged = false;
	Log->GridColumns = 13;
	Log->GridCellSize = 100.0f;

	// Place frog on a river row, riding the log
	Frog->GridPosition = FIntPoint(6, 7);
	Frog->CurrentPlatform = Log;
	Frog->SetActorLocation(FVector(600.0f, 700.0f, 0.0f));

	// Simulate 1 second of riding
	Frog->UpdateRiding(1.0f);

	FVector Pos = Frog->GetActorLocation();
	TestNearlyEqual(TEXT("Frog X moved with platform"), Pos.X, 700.0);
	TestNearlyEqual(TEXT("Frog Y unchanged"), Pos.Y, 700.0);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Submerged turtle is NOT rideable -- frog falls through
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCollisionSystem_SubmergedTurtleNotRideable,
	"UnrealFrog.Collision.SubmergedTurtleNotRideable",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCollisionSystem_SubmergedTurtleNotRideable::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	AHazardBase* Turtle = NewObject<AHazardBase>();

	// Configure a submerged turtle
	Turtle->HazardType = EHazardType::TurtleGroup;
	Turtle->bIsRideable = true;
	Turtle->bIsSubmerged = true;
	Turtle->Speed = 80.0f;
	Turtle->bMovesRight = true;
	Turtle->GridColumns = 13;
	Turtle->GridCellSize = 100.0f;

	// Frog is on a river row with a submerged turtle as "platform"
	Frog->GridPosition = FIntPoint(6, 9);
	Frog->CurrentPlatform = Turtle;

	// A submerged turtle should not count as rideable
	TestTrue(TEXT("Submerged turtle triggers river death check"), Frog->CheckRiverDeath());

	return true;
}

// ---------------------------------------------------------------------------
// Test: Frog carried off screen edge triggers OffScreen detection
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCollisionSystem_OffScreenDeath,
	"UnrealFrog.Collision.OffScreenDeath",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCollisionSystem_OffScreenDeath::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();

	// Grid is 13 columns * 100 UU = 1300 UU wide (X range 0 to 1300)
	Frog->GridColumns = 13;
	Frog->GridCellSize = 100.0f;

	// Frog at right edge -- still valid
	Frog->SetActorLocation(FVector(1200.0f, 700.0f, 0.0f));
	TestFalse(TEXT("Frog at right edge is on screen"), Frog->IsOffScreen());

	// Frog past right edge
	Frog->SetActorLocation(FVector(1350.0f, 700.0f, 0.0f));
	TestTrue(TEXT("Frog past right edge is off screen"), Frog->IsOffScreen());

	// Frog past left edge
	Frog->SetActorLocation(FVector(-50.0f, 700.0f, 0.0f));
	TestTrue(TEXT("Frog past left edge is off screen"), Frog->IsOffScreen());

	return true;
}

// ---------------------------------------------------------------------------
// Test: Frog respawns at start position (6, 0) after death
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCollisionSystem_RespawnAfterDeath,
	"UnrealFrog.Collision.RespawnAfterDeath",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCollisionSystem_RespawnAfterDeath::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();

	// Move frog to a non-start position and kill it
	Frog->GridPosition = FIntPoint(3, 8);
	Frog->Die(EDeathType::Splash);

	TestTrue(TEXT("Frog is dead"), Frog->bIsDead);

	// Directly call Respawn (in-game this would be triggered by a timer)
	Frog->Respawn();

	TestFalse(TEXT("Frog is alive after respawn"), Frog->bIsDead);
	TestEqual(TEXT("Grid X reset to 6"), Frog->GridPosition.X, 6);
	TestEqual(TEXT("Grid Y reset to 0"), Frog->GridPosition.Y, 0);

	return true;
}

// ---------------------------------------------------------------------------
// Test: No respawn when game is over (0 lives)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCollisionSystem_NoRespawnOnGameOver,
	"UnrealFrog.Collision.NoRespawnOnGameOver",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCollisionSystem_NoRespawnOnGameOver::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();

	// Frog should know if game is over via ShouldRespawn check
	Frog->bIsGameOver = true;
	Frog->GridPosition = FIntPoint(3, 5);
	Frog->Die(EDeathType::Squish);

	TestTrue(TEXT("Frog is dead"), Frog->bIsDead);
	TestFalse(TEXT("Should not respawn when game over"), Frog->ShouldRespawn());

	return true;
}

#endif // WITH_AUTOMATION_TESTS
