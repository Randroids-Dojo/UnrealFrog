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

// ===========================================================================
// FindPlatformAtCurrentPosition boundary tests
//
// Sprint 11, Task #4
//
// These tests exercise the boundary conditions of the synchronous platform
// detection in FindPlatformAtCurrentPosition(). Each test spawns a frog and
// a hazard in a real UWorld, positions the frog at a specific offset from
// the hazard center, and calls FindPlatformAtCurrentPosition() to verify
// the detection result.
//
// The detection formula (FrogCharacter.cpp):
//   HalfWidth = HazardWidthCells * GridCellSize * 0.5
//   EffectiveHalfWidth = HalfWidth - PlatformLandingMargin
//   FOUND if: abs(FrogX - HazardX) <= EffectiveHalfWidth
//         AND abs(FrogY - HazardY) <= HalfCell
//
// All tests read PlatformLandingMargin from the frog at runtime
// (tuning-resilient per Agreement Section 2).
// ===========================================================================

namespace FindPlatformTestHelper
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

	AFrogCharacter* SpawnFrogAtWorldPos(UWorld* World, FVector WorldPos, int32 RiverRow)
	{
		FTransform SpawnTransform(FRotator::ZeroRotator, WorldPos);
		AFrogCharacter* Frog = World->SpawnActor<AFrogCharacter>(
			AFrogCharacter::StaticClass(), SpawnTransform);
		if (Frog)
		{
			Frog->GridPosition = FIntPoint(
				FMath::RoundToInt(WorldPos.X / Frog->GridCellSize), RiverRow);
		}
		return Frog;
	}

	AHazardBase* SpawnLog(UWorld* World, FVector Pos, int32 WidthCells, float GridCellSize)
	{
		FTransform SpawnTransform(FRotator::ZeroRotator, Pos);
		AHazardBase* Log = World->SpawnActor<AHazardBase>(
			AHazardBase::StaticClass(), SpawnTransform);
		if (Log)
		{
			Log->bIsRideable = true;
			Log->HazardType = EHazardType::SmallLog;
			Log->HazardWidthCells = WidthCells;
			Log->Speed = 100.0f;
			Log->BaseSpeed = 100.0f;
			Log->bMovesRight = true;
			Log->GridCellSize = GridCellSize;
		}
		return Log;
	}
}

// Test 1: Dead center on platform -> FOUND
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFindPlatform_DeadCenter_Found,
	"UnrealFrog.Collision.FindPlatform.DeadCenter_Found",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFindPlatform_DeadCenter_Found::RunTest(const FString& Parameters)
{
	UWorld* World = FindPlatformTestHelper::CreateTestWorld();
	float CellSize = 100.0f;
	int32 RiverRow = 8;
	float LogX = 600.0f;
	float LogY = static_cast<float>(RiverRow) * CellSize;

	AHazardBase* Log = FindPlatformTestHelper::SpawnLog(World, FVector(LogX, LogY, 0.0f), 3, CellSize);
	AFrogCharacter* Frog = FindPlatformTestHelper::SpawnFrogAtWorldPos(World, FVector(LogX, LogY, 0.0f), RiverRow);
	TestNotNull(TEXT("Frog spawned"), Frog);
	TestNotNull(TEXT("Log spawned"), Log);

	if (Frog)
	{
		Frog->FindPlatformAtCurrentPosition();
		TestTrue(TEXT("Dead center: platform FOUND"), Frog->CurrentPlatform.Get() == Log);
	}

	FindPlatformTestHelper::DestroyTestWorld(World);
	return true;
}

// Test 2: Comfortable interior (25% offset) -> FOUND
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFindPlatform_Interior25Pct_Found,
	"UnrealFrog.Collision.FindPlatform.Interior25Pct_Found",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFindPlatform_Interior25Pct_Found::RunTest(const FString& Parameters)
{
	UWorld* World = FindPlatformTestHelper::CreateTestWorld();
	float CellSize = 100.0f;
	int32 RiverRow = 8;
	float LogX = 600.0f;
	float LogY = static_cast<float>(RiverRow) * CellSize;

	AHazardBase* Log = FindPlatformTestHelper::SpawnLog(World, FVector(LogX, LogY, 0.0f), 3, CellSize);
	AFrogCharacter* Frog = FindPlatformTestHelper::SpawnFrogAtWorldPos(World, FVector(LogX, LogY, 0.0f), RiverRow);
	TestNotNull(TEXT("Frog spawned"), Frog);

	if (Frog && Log)
	{
		float Margin = Frog->PlatformLandingMargin;
		float HalfWidth = 3.0f * CellSize * 0.5f;
		float EffectiveHalfWidth = HalfWidth - Margin;
		float Offset = EffectiveHalfWidth * 0.25f;

		Frog->SetActorLocation(FVector(LogX + Offset, LogY, 0.0f));
		Frog->FindPlatformAtCurrentPosition();
		TestTrue(TEXT("25% interior: platform FOUND"), Frog->CurrentPlatform.Get() == Log);
	}

	FindPlatformTestHelper::DestroyTestWorld(World);
	return true;
}

// Test 3: Just inside margin (EffectiveHalfWidth - 1.0) -> FOUND
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFindPlatform_JustInsideMargin_Found,
	"UnrealFrog.Collision.FindPlatform.JustInsideMargin_Found",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFindPlatform_JustInsideMargin_Found::RunTest(const FString& Parameters)
{
	UWorld* World = FindPlatformTestHelper::CreateTestWorld();
	float CellSize = 100.0f;
	int32 RiverRow = 8;
	float LogX = 600.0f;
	float LogY = static_cast<float>(RiverRow) * CellSize;

	AHazardBase* Log = FindPlatformTestHelper::SpawnLog(World, FVector(LogX, LogY, 0.0f), 3, CellSize);
	AFrogCharacter* Frog = FindPlatformTestHelper::SpawnFrogAtWorldPos(World, FVector(LogX, LogY, 0.0f), RiverRow);
	TestNotNull(TEXT("Frog spawned"), Frog);

	if (Frog && Log)
	{
		float Margin = Frog->PlatformLandingMargin;
		float HalfWidth = 3.0f * CellSize * 0.5f;
		float EffectiveHalfWidth = HalfWidth - Margin;
		float Offset = EffectiveHalfWidth - 1.0f;

		Frog->SetActorLocation(FVector(LogX + Offset, LogY, 0.0f));
		Frog->FindPlatformAtCurrentPosition();
		TestTrue(TEXT("Just inside margin: platform FOUND"), Frog->CurrentPlatform.Get() == Log);
	}

	FindPlatformTestHelper::DestroyTestWorld(World);
	return true;
}

// Test 4: Just outside margin (EffectiveHalfWidth + 1.0) -> NOT FOUND
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFindPlatform_JustOutsideMargin_NotFound,
	"UnrealFrog.Collision.FindPlatform.JustOutsideMargin_NotFound",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFindPlatform_JustOutsideMargin_NotFound::RunTest(const FString& Parameters)
{
	UWorld* World = FindPlatformTestHelper::CreateTestWorld();
	float CellSize = 100.0f;
	int32 RiverRow = 8;
	float LogX = 600.0f;
	float LogY = static_cast<float>(RiverRow) * CellSize;

	AHazardBase* Log = FindPlatformTestHelper::SpawnLog(World, FVector(LogX, LogY, 0.0f), 3, CellSize);
	AFrogCharacter* Frog = FindPlatformTestHelper::SpawnFrogAtWorldPos(World, FVector(LogX, LogY, 0.0f), RiverRow);
	TestNotNull(TEXT("Frog spawned"), Frog);

	if (Frog && Log)
	{
		float Margin = Frog->PlatformLandingMargin;
		float HalfWidth = 3.0f * CellSize * 0.5f;
		float EffectiveHalfWidth = HalfWidth - Margin;
		float Offset = EffectiveHalfWidth + 1.0f;

		Frog->SetActorLocation(FVector(LogX + Offset, LogY, 0.0f));
		Frog->FindPlatformAtCurrentPosition();
		TestTrue(TEXT("Just outside margin: platform NOT found"), Frog->CurrentPlatform.Get() == nullptr);
	}

	FindPlatformTestHelper::DestroyTestWorld(World);
	return true;
}

// Test 5: At raw platform edge (HalfWidth) -> NOT FOUND
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFindPlatform_AtRawEdge_NotFound,
	"UnrealFrog.Collision.FindPlatform.AtRawEdge_NotFound",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFindPlatform_AtRawEdge_NotFound::RunTest(const FString& Parameters)
{
	UWorld* World = FindPlatformTestHelper::CreateTestWorld();
	float CellSize = 100.0f;
	int32 RiverRow = 8;
	float LogX = 600.0f;
	float LogY = static_cast<float>(RiverRow) * CellSize;

	AHazardBase* Log = FindPlatformTestHelper::SpawnLog(World, FVector(LogX, LogY, 0.0f), 3, CellSize);
	AFrogCharacter* Frog = FindPlatformTestHelper::SpawnFrogAtWorldPos(World, FVector(LogX, LogY, 0.0f), RiverRow);
	TestNotNull(TEXT("Frog spawned"), Frog);

	if (Frog && Log)
	{
		float HalfWidth = 3.0f * CellSize * 0.5f;
		Frog->SetActorLocation(FVector(LogX + HalfWidth, LogY, 0.0f));
		Frog->FindPlatformAtCurrentPosition();
		TestTrue(TEXT("At raw edge: platform NOT found"), Frog->CurrentPlatform.Get() == nullptr);
	}

	FindPlatformTestHelper::DestroyTestWorld(World);
	return true;
}

// Test 6: Wrong row (Y off by 1 cell) -> NOT FOUND
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFindPlatform_WrongRow_NotFound,
	"UnrealFrog.Collision.FindPlatform.WrongRow_NotFound",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFindPlatform_WrongRow_NotFound::RunTest(const FString& Parameters)
{
	UWorld* World = FindPlatformTestHelper::CreateTestWorld();
	float CellSize = 100.0f;
	int32 RiverRow = 8;
	float LogX = 600.0f;
	float LogY = static_cast<float>(RiverRow) * CellSize;

	AHazardBase* Log = FindPlatformTestHelper::SpawnLog(World, FVector(LogX, LogY, 0.0f), 3, CellSize);
	int32 AdjacentRow = RiverRow + 1;
	AFrogCharacter* Frog = FindPlatformTestHelper::SpawnFrogAtWorldPos(
		World, FVector(LogX, LogY + CellSize, 0.0f), AdjacentRow);
	TestNotNull(TEXT("Frog spawned"), Frog);

	if (Frog && Log)
	{
		Frog->FindPlatformAtCurrentPosition();
		TestTrue(TEXT("Wrong row: platform NOT found"), Frog->CurrentPlatform.Get() == nullptr);
	}

	FindPlatformTestHelper::DestroyTestWorld(World);
	return true;
}

// Test 7: Non-rideable hazard -> NOT FOUND
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFindPlatform_NonRideable_NotFound,
	"UnrealFrog.Collision.FindPlatform.NonRideable_NotFound",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFindPlatform_NonRideable_NotFound::RunTest(const FString& Parameters)
{
	UWorld* World = FindPlatformTestHelper::CreateTestWorld();
	float CellSize = 100.0f;
	int32 RiverRow = 8;
	float HazardX = 600.0f;
	float HazardY = static_cast<float>(RiverRow) * CellSize;

	FTransform CarTransform(FRotator::ZeroRotator, FVector(HazardX, HazardY, 0.0f));
	AHazardBase* Car = World->SpawnActor<AHazardBase>(AHazardBase::StaticClass(), CarTransform);
	if (Car)
	{
		Car->bIsRideable = false;
		Car->HazardType = EHazardType::Car;
		Car->HazardWidthCells = 2;
		Car->GridCellSize = CellSize;
	}

	AFrogCharacter* Frog = FindPlatformTestHelper::SpawnFrogAtWorldPos(
		World, FVector(HazardX, HazardY, 0.0f), RiverRow);
	TestNotNull(TEXT("Frog spawned"), Frog);

	if (Frog && Car)
	{
		Frog->FindPlatformAtCurrentPosition();
		TestTrue(TEXT("Non-rideable: platform NOT found"), Frog->CurrentPlatform.Get() == nullptr);
	}

	FindPlatformTestHelper::DestroyTestWorld(World);
	return true;
}

// Test 8: Two platforms, frog on first -> FOUND (first)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFindPlatform_TwoPlatforms_FindsCorrectOne,
	"UnrealFrog.Collision.FindPlatform.TwoPlatforms_FindsCorrectOne",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFindPlatform_TwoPlatforms_FindsCorrectOne::RunTest(const FString& Parameters)
{
	UWorld* World = FindPlatformTestHelper::CreateTestWorld();
	float CellSize = 100.0f;
	int32 RiverRow = 8;
	float LogY = static_cast<float>(RiverRow) * CellSize;

	AHazardBase* LogA = FindPlatformTestHelper::SpawnLog(World, FVector(400.0f, LogY, 0.0f), 3, CellSize);
	AHazardBase* LogB = FindPlatformTestHelper::SpawnLog(World, FVector(900.0f, LogY, 0.0f), 3, CellSize);
	AFrogCharacter* Frog = FindPlatformTestHelper::SpawnFrogAtWorldPos(World, FVector(400.0f, LogY, 0.0f), RiverRow);
	TestNotNull(TEXT("Frog spawned"), Frog);
	TestNotNull(TEXT("Log A spawned"), LogA);
	TestNotNull(TEXT("Log B spawned"), LogB);

	if (Frog && LogA && LogB)
	{
		Frog->FindPlatformAtCurrentPosition();
		AHazardBase* FoundPlatform = Frog->CurrentPlatform.Get();
		TestNotNull(TEXT("A platform was found"), FoundPlatform);
		TestTrue(TEXT("Found platform is Log A"), FoundPlatform == LogA);
	}

	FindPlatformTestHelper::DestroyTestWorld(World);
	return true;
}

// Test 9: Negative X offset just inside -> FOUND
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFindPlatform_NegativeSideJustInside_Found,
	"UnrealFrog.Collision.FindPlatform.NegativeSideJustInside_Found",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFindPlatform_NegativeSideJustInside_Found::RunTest(const FString& Parameters)
{
	UWorld* World = FindPlatformTestHelper::CreateTestWorld();
	float CellSize = 100.0f;
	int32 RiverRow = 8;
	float LogX = 600.0f;
	float LogY = static_cast<float>(RiverRow) * CellSize;

	AHazardBase* Log = FindPlatformTestHelper::SpawnLog(World, FVector(LogX, LogY, 0.0f), 3, CellSize);
	AFrogCharacter* Frog = FindPlatformTestHelper::SpawnFrogAtWorldPos(World, FVector(LogX, LogY, 0.0f), RiverRow);
	TestNotNull(TEXT("Frog spawned"), Frog);

	if (Frog && Log)
	{
		float Margin = Frog->PlatformLandingMargin;
		float HalfWidth = 3.0f * CellSize * 0.5f;
		float EffectiveHalfWidth = HalfWidth - Margin;
		float Offset = -(EffectiveHalfWidth - 1.0f);

		Frog->SetActorLocation(FVector(LogX + Offset, LogY, 0.0f));
		Frog->FindPlatformAtCurrentPosition();
		TestTrue(TEXT("Negative side just inside: platform FOUND"), Frog->CurrentPlatform.Get() == Log);
	}

	FindPlatformTestHelper::DestroyTestWorld(World);
	return true;
}

// Test 10: Submerged platform -> NOT FOUND
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFindPlatform_SubmergedPlatform_NotFound,
	"UnrealFrog.Collision.FindPlatform.SubmergedPlatform_NotFound",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFindPlatform_SubmergedPlatform_NotFound::RunTest(const FString& Parameters)
{
	UWorld* World = FindPlatformTestHelper::CreateTestWorld();
	float CellSize = 100.0f;
	int32 RiverRow = 8;
	float LogX = 600.0f;
	float LogY = static_cast<float>(RiverRow) * CellSize;

	AHazardBase* Turtle = FindPlatformTestHelper::SpawnLog(World, FVector(LogX, LogY, 0.0f), 3, CellSize);
	if (Turtle)
	{
		Turtle->HazardType = EHazardType::TurtleGroup;
		Turtle->bIsSubmerged = true;
	}

	AFrogCharacter* Frog = FindPlatformTestHelper::SpawnFrogAtWorldPos(World, FVector(LogX, LogY, 0.0f), RiverRow);
	TestNotNull(TEXT("Frog spawned"), Frog);

	if (Frog && Turtle)
	{
		Frog->FindPlatformAtCurrentPosition();
		TestTrue(TEXT("Submerged platform: NOT found"), Frog->CurrentPlatform.Get() == nullptr);
	}

	FindPlatformTestHelper::DestroyTestWorld(World);
	return true;
}

#endif // WITH_AUTOMATION_TESTS
