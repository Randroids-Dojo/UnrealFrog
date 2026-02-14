// Copyright UnrealFrog Team. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Core/FrogCharacter.h"
#include "Core/HazardBase.h"

#if WITH_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Test: Road hazard overlap triggers Die(Squish)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCollision_RoadHazardKillsFrog,
	"UnrealFrog.Collision.RoadHazardKillsFrog",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCollision_RoadHazardKillsFrog::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	AHazardBase* Car = NewObject<AHazardBase>();
	Car->HazardType = EHazardType::Car;
	Car->bIsRideable = false;

	TestFalse(TEXT("Frog starts alive"), Frog->bIsDead);

	Frog->HandleHazardOverlap(Car);

	TestTrue(TEXT("Frog should be dead"), Frog->bIsDead);
	TestEqual(TEXT("Death type should be Squish"), Frog->LastDeathType, EDeathType::Squish);

	return true;
}

// ---------------------------------------------------------------------------
// Test: River platform overlap mounts frog on platform
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCollision_RiverPlatformMountsFrog,
	"UnrealFrog.Collision.RiverPlatformMountsFrog",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCollision_RiverPlatformMountsFrog::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	AHazardBase* Log = NewObject<AHazardBase>();
	Log->HazardType = EHazardType::SmallLog;
	Log->bIsRideable = true;

	TestTrue(TEXT("No platform initially"), Frog->CurrentPlatform.Get() == nullptr);

	Frog->HandleHazardOverlap(Log);

	TestFalse(TEXT("Frog should not be dead"), Frog->bIsDead);
	TestTrue(TEXT("Current platform should be the log"), Frog->CurrentPlatform.Get() == Log);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Platform end overlap clears CurrentPlatform
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCollision_PlatformEndOverlapClearsPlatform,
	"UnrealFrog.Collision.PlatformEndOverlapClears",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCollision_PlatformEndOverlapClearsPlatform::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	AHazardBase* Log = NewObject<AHazardBase>();
	Log->bIsRideable = true;

	Frog->HandleHazardOverlap(Log);
	TestTrue(TEXT("Platform is set"), Frog->CurrentPlatform.Get() == Log);

	Frog->HandlePlatformEndOverlap(Log);
	TestTrue(TEXT("Platform cleared after end overlap"), Frog->CurrentPlatform.Get() == nullptr);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Overlap is ignored when frog is already dead
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCollision_OverlapIgnoredWhenDead,
	"UnrealFrog.Collision.OverlapIgnoredWhenDead",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCollision_OverlapIgnoredWhenDead::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	AHazardBase* Car = NewObject<AHazardBase>();
	Car->bIsRideable = false;
	AHazardBase* Log = NewObject<AHazardBase>();
	Log->bIsRideable = true;

	// Kill the frog
	Frog->Die(EDeathType::Squish);
	TestTrue(TEXT("Frog is dead"), Frog->bIsDead);

	// Overlap with a rideable platform should be ignored — no mounting
	Frog->HandleHazardOverlap(Log);
	TestTrue(TEXT("Platform not set while dead"), Frog->CurrentPlatform.Get() == nullptr);

	return true;
}

// ---------------------------------------------------------------------------
// Test: End overlap for a different platform does not clear current platform
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCollision_EndOverlapDifferentPlatform,
	"UnrealFrog.Collision.EndOverlapDifferentPlatform",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCollision_EndOverlapDifferentPlatform::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	AHazardBase* Log1 = NewObject<AHazardBase>();
	Log1->bIsRideable = true;
	AHazardBase* Log2 = NewObject<AHazardBase>();
	Log2->bIsRideable = true;

	Frog->HandleHazardOverlap(Log1);
	TestTrue(TEXT("Mounted on Log1"), Frog->CurrentPlatform.Get() == Log1);

	// End overlap with a different log should not clear current platform
	Frog->HandlePlatformEndOverlap(Log2);
	TestTrue(TEXT("Still on Log1"), Frog->CurrentPlatform.Get() == Log1);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Turtle submerge cycle timing (Wave 2+)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCollision_TurtleSubmergeCycle,
	"UnrealFrog.Collision.TurtleSubmergeCycle",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCollision_TurtleSubmergeCycle::RunTest(const FString& Parameters)
{
	AHazardBase* Turtle = NewObject<AHazardBase>();
	Turtle->HazardType = EHazardType::TurtleGroup;
	Turtle->bIsRideable = true;
	Turtle->CurrentWave = 2; // Wave 2+: submerge enabled
	Turtle->SurfaceDuration = 4.0f;
	Turtle->WarningDuration = 1.0f;
	Turtle->SubmergeDuration = 2.0f;

	// Starts surfaced
	TestFalse(TEXT("Starts surfaced"), Turtle->bIsSubmerged);
	TestEqual(TEXT("Starts in Surface phase"), Turtle->SubmergePhase, ESubmergePhase::Surface);

	// Tick through surface phase (4.0s)
	Turtle->TickSubmerge(3.9f);
	TestEqual(TEXT("Still Surface at 3.9s"), Turtle->SubmergePhase, ESubmergePhase::Surface);

	Turtle->TickSubmerge(0.2f); // 4.1s total → warning
	TestEqual(TEXT("Warning at 4.1s"), Turtle->SubmergePhase, ESubmergePhase::Warning);
	TestFalse(TEXT("Not submerged during warning"), Turtle->bIsSubmerged);

	// Tick through warning phase (1.0s)
	Turtle->TickSubmerge(0.9f);
	TestEqual(TEXT("Still Warning at 5.0s"), Turtle->SubmergePhase, ESubmergePhase::Warning);

	Turtle->TickSubmerge(0.2f); // 5.2s → submerged
	TestEqual(TEXT("Submerged at 5.2s"), Turtle->SubmergePhase, ESubmergePhase::Submerged);
	TestTrue(TEXT("bIsSubmerged true"), Turtle->bIsSubmerged);

	// Tick through submerge phase (2.0s)
	Turtle->TickSubmerge(1.9f);
	TestEqual(TEXT("Still Submerged at 7.1s"), Turtle->SubmergePhase, ESubmergePhase::Submerged);

	Turtle->TickSubmerge(0.2f); // 7.3s → back to surface
	TestEqual(TEXT("Back to Surface at 7.3s"), Turtle->SubmergePhase, ESubmergePhase::Surface);
	TestFalse(TEXT("bIsSubmerged false"), Turtle->bIsSubmerged);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Wave 1 turtles never submerge
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCollision_Wave1TurtlesNeverSubmerge,
	"UnrealFrog.Collision.Wave1TurtlesNeverSubmerge",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCollision_Wave1TurtlesNeverSubmerge::RunTest(const FString& Parameters)
{
	AHazardBase* Turtle = NewObject<AHazardBase>();
	Turtle->HazardType = EHazardType::TurtleGroup;
	Turtle->bIsRideable = true;
	Turtle->CurrentWave = 1; // Wave 1: no submerge
	Turtle->SurfaceDuration = 4.0f;
	Turtle->WarningDuration = 1.0f;
	Turtle->SubmergeDuration = 2.0f;

	// Tick well past the full cycle
	Turtle->TickSubmerge(20.0f);

	TestFalse(TEXT("Wave 1 turtle never submerges"), Turtle->bIsSubmerged);
	TestEqual(TEXT("Phase stays Surface"), Turtle->SubmergePhase, ESubmergePhase::Surface);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Frog on submerged turtle dies
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCollision_FrogDiesOnSubmergedTurtle,
	"UnrealFrog.Collision.FrogDiesOnSubmergedTurtle",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCollision_FrogDiesOnSubmergedTurtle::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	AHazardBase* Turtle = NewObject<AHazardBase>();
	Turtle->HazardType = EHazardType::TurtleGroup;
	Turtle->bIsRideable = true;
	Turtle->bIsSubmerged = true;

	// Frog is on a river row
	Frog->GridPosition = FIntPoint(6, 8);
	Frog->CurrentPlatform = Turtle;

	// CheckRiverDeath should return true when platform is submerged
	TestTrue(TEXT("River death on submerged turtle"), Frog->CheckRiverDeath());

	return true;
}

// ---------------------------------------------------------------------------
// Test: Frog on surfaced turtle survives
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCollision_FrogSurvivesOnSurfacedTurtle,
	"UnrealFrog.Collision.FrogSurvivesOnSurfacedTurtle",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCollision_FrogSurvivesOnSurfacedTurtle::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	AHazardBase* Turtle = NewObject<AHazardBase>();
	Turtle->HazardType = EHazardType::TurtleGroup;
	Turtle->bIsRideable = true;
	Turtle->bIsSubmerged = false;

	// Frog is on a river row with the turtle
	Frog->GridPosition = FIntPoint(6, 8);
	Frog->CurrentPlatform = Turtle;

	// CheckRiverDeath should return false when platform is surfaced
	TestFalse(TEXT("No river death on surfaced turtle"), Frog->CheckRiverDeath());

	return true;
}

#endif // WITH_AUTOMATION_TESTS
