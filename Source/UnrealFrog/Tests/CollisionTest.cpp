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

#endif // WITH_AUTOMATION_TESTS
