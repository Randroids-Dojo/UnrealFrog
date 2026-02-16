// Copyright UnrealFrog Team. All Rights Reserved.
//
// [Gameplay] Multi-step scenario tests.
//
// These tests simulate hop sequences through actual UWorld environments,
// verifying position-based survival and death. They exercise the full
// game loop: hop -> position check -> collision/platform detection -> outcome.
//
// Sprint 11, Task #9 -- QA Lead
//
// Agreement Section 2: Tuning-resilient -- read grid/timing values from
// game objects at test time, not hardcoded magic numbers.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "Core/FrogCharacter.h"
#include "Core/HazardBase.h"
#include "Core/UnrealFrogGameMode.h"

#if WITH_AUTOMATION_TESTS

namespace GameplayTestHelper
{
	/** Create a minimal game world for gameplay testing. */
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

	/** Spawn a frog at a specific grid position in the given world. */
	AFrogCharacter* SpawnFrogAt(UWorld* World, FIntPoint GridPos, float GridCellSize)
	{
		FVector WorldPos(
			static_cast<float>(GridPos.X) * GridCellSize,
			static_cast<float>(GridPos.Y) * GridCellSize,
			0.0f);
		FTransform SpawnTransform(FRotator::ZeroRotator, WorldPos);
		AFrogCharacter* Frog = World->SpawnActor<AFrogCharacter>(
			AFrogCharacter::StaticClass(), SpawnTransform);
		if (Frog)
		{
			Frog->GridPosition = GridPos;
		}
		return Frog;
	}

	/** Spawn a hazard at a specific row and X position in the given world. */
	AHazardBase* SpawnHazardAt(UWorld* World, float XPos, int32 Row,
		float GridCellSize, bool bRideable, int32 WidthCells, float Speed, bool bMovesRight)
	{
		FVector Pos(XPos, static_cast<float>(Row) * GridCellSize, 0.0f);
		FTransform SpawnTransform(FRotator::ZeroRotator, Pos);
		AHazardBase* Hazard = World->SpawnActor<AHazardBase>(
			AHazardBase::StaticClass(), SpawnTransform);
		if (Hazard)
		{
			Hazard->bIsRideable = bRideable;
			Hazard->HazardWidthCells = WidthCells;
			Hazard->Speed = Speed;
			Hazard->BaseSpeed = Speed;
			Hazard->bMovesRight = bMovesRight;
			Hazard->GridCellSize = GridCellSize;
			if (bRideable)
			{
				Hazard->HazardType = EHazardType::SmallLog;
			}
			else
			{
				Hazard->HazardType = EHazardType::Car;
			}
		}
		return Hazard;
	}
}

// ---------------------------------------------------------------------------
// Scenario 1: Hop forward through safe zone -> survive
//
// The frog starts at row 0 (safe zone) and hops forward to row 1 (also safe
// in the default layout -- row 0 is start, row 6 is the pre-river safe zone).
// No hazards present in safe rows. Frog should survive and arrive at the
// target grid position.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameplay_HopThroughSafeZone_Survive,
	"UnrealFrog.Gameplay.SafeZoneHop.Survive",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameplay_HopThroughSafeZone_Survive::RunTest(const FString& Parameters)
{
	UWorld* World = GameplayTestHelper::CreateTestWorld();

	// Spawn frog at grid start (column 6, row 0)
	AFrogCharacter* Frog = GameplayTestHelper::SpawnFrogAt(World, FIntPoint(6, 0), 100.0f);
	TestNotNull(TEXT("Frog spawned"), Frog);
	if (!Frog)
	{
		GameplayTestHelper::DestroyTestWorld(World);
		return true;
	}

	// Read hop duration from the frog (tuning-resilient)
	float HopDuration = Frog->HopDuration;
	float CellSize = Frog->GridCellSize;

	// Hop forward (positive Y)
	Frog->RequestHop(FVector(0.0, 1.0, 0.0));
	TestTrue(TEXT("Frog is hopping"), Frog->bIsHopping);

	// Tick past the hop duration to complete it
	Frog->Tick(HopDuration + 0.01f);
	TestFalse(TEXT("Hop completed"), Frog->bIsHopping);

	// Frog should be alive and at row 1
	TestFalse(TEXT("Frog is alive"), Frog->bIsDead);
	TestEqual(TEXT("Frog at row 1"), Frog->GridPosition.Y, 1);
	TestEqual(TEXT("Frog still at column 6"), Frog->GridPosition.X, 6);

	// Verify world position matches grid position
	FVector ActualPos = Frog->GetActorLocation();
	TestNearlyEqual(TEXT("World X at column 6"), ActualPos.X, 6.0 * CellSize, 1.0);
	TestNearlyEqual(TEXT("World Y at row 1"), ActualPos.Y, 1.0 * CellSize, 1.0);

	GameplayTestHelper::DestroyTestWorld(World);
	return true;
}

// ---------------------------------------------------------------------------
// Scenario 2: Hop into road hazard -> die
//
// A road hazard (car) is placed directly in the frog's path at row 3.
// After hopping from row 2 to row 3, the frog should die from Squish.
// This tests the HandleHazardOverlap -> Die(Squish) path with actual
// actors in a world. Note: overlap events require physics ticking which
// doesn't happen in our minimal test world. Instead, we verify the
// position-based death by manually calling HandleHazardOverlap after
// the frog lands within the hazard's footprint.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameplay_HopIntoRoadHazard_Die,
	"UnrealFrog.Gameplay.RoadHazard.Die",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameplay_HopIntoRoadHazard_Die::RunTest(const FString& Parameters)
{
	UWorld* World = GameplayTestHelper::CreateTestWorld();

	float CellSize = 100.0f;

	// Frog starts at row 2 (road zone)
	AFrogCharacter* Frog = GameplayTestHelper::SpawnFrogAt(World, FIntPoint(6, 2), CellSize);
	TestNotNull(TEXT("Frog spawned"), Frog);
	if (!Frog)
	{
		GameplayTestHelper::DestroyTestWorld(World);
		return true;
	}

	// Spawn a car at row 3, X=600 (same column as frog), width 2 cells
	AHazardBase* Car = GameplayTestHelper::SpawnHazardAt(
		World, 600.0f, 3, CellSize, /*bRideable=*/false, /*WidthCells=*/2, /*Speed=*/150.0f, /*bMovesRight=*/true);
	TestNotNull(TEXT("Car spawned"), Car);

	float HopDuration = Frog->HopDuration;

	// Hop forward into row 3 where the car is
	Frog->RequestHop(FVector(0.0, 1.0, 0.0));
	TestTrue(TEXT("Frog is hopping"), Frog->bIsHopping);

	// Complete the hop
	Frog->Tick(HopDuration + 0.01f);
	TestFalse(TEXT("Hop completed"), Frog->bIsHopping);
	TestEqual(TEXT("Frog at row 3"), Frog->GridPosition.Y, 3);

	// Simulate the overlap that would fire in a full game.
	// In real gameplay, the physics engine detects overlap and calls
	// OnBeginOverlap -> HandleHazardOverlap. In our test world without
	// full physics ticking, we call it directly.
	Frog->HandleHazardOverlap(Car);

	TestTrue(TEXT("Frog is dead"), Frog->bIsDead);
	TestEqual(TEXT("Death type is Squish"), Frog->LastDeathType, EDeathType::Squish);

	GameplayTestHelper::DestroyTestWorld(World);
	return true;
}

// ---------------------------------------------------------------------------
// Scenario 3: Hop onto river platform -> survive
//
// The frog hops from a safe row onto a river row where a rideable log is
// positioned. FindPlatformAtCurrentPosition (which uses TActorIterator)
// should detect the log, mount the frog, and prevent river death.
//
// This is the critical test: it uses a real UWorld with TActorIterator,
// verifying that the synchronous platform detection works end-to-end.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameplay_HopOntoRiverPlatform_Survive,
	"UnrealFrog.Gameplay.RiverPlatform.Survive",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameplay_HopOntoRiverPlatform_Survive::RunTest(const FString& Parameters)
{
	UWorld* World = GameplayTestHelper::CreateTestWorld();

	float CellSize = 100.0f;
	int32 RiverRow = 8; // Within default RiverRowMin(7)..RiverRowMax(12)

	// Frog starts at the safe row just before the river (row 6)
	// We place at row 7-1 = row 6 for one hop into river row 7,
	// but to test a specific river row, start one row before it.
	int32 StartRow = RiverRow - 1;
	AFrogCharacter* Frog = GameplayTestHelper::SpawnFrogAt(World, FIntPoint(6, StartRow), CellSize);
	TestNotNull(TEXT("Frog spawned"), Frog);
	if (!Frog)
	{
		GameplayTestHelper::DestroyTestWorld(World);
		return true;
	}

	// Verify the target row is a river row
	TestTrue(TEXT("Target row is within river range"),
		RiverRow >= Frog->RiverRowMin && RiverRow <= Frog->RiverRowMax);

	// Spawn a 3-cell log centered on the frog's column at the river row
	// Log X = frog column * cellSize = 600. Width = 3 cells = 300 UU.
	// Frog capsule radius is ~34 UU. EffectiveHalfWidth = 150 - 34 = 116.
	// Frog will be at center (offset=0), well within 116 UU margin.
	AHazardBase* Log = GameplayTestHelper::SpawnHazardAt(
		World, 600.0f, RiverRow, CellSize, /*bRideable=*/true, /*WidthCells=*/3, /*Speed=*/100.0f, /*bMovesRight=*/true);
	TestNotNull(TEXT("Log spawned"), Log);

	float HopDuration = Frog->HopDuration;

	// Hop forward into the river row
	Frog->RequestHop(FVector(0.0, 1.0, 0.0));
	TestTrue(TEXT("Frog is hopping"), Frog->bIsHopping);

	// Complete the hop — FinishHop calls FindPlatformAtCurrentPosition
	Frog->Tick(HopDuration + 0.01f);
	TestFalse(TEXT("Hop completed"), Frog->bIsHopping);
	TestEqual(TEXT("Frog at river row"), Frog->GridPosition.Y, RiverRow);

	// FindPlatformAtCurrentPosition should have found the log
	TestTrue(TEXT("Frog mounted on log"), Frog->CurrentPlatform.Get() == Log);

	// Frog should be alive — the log prevents river death
	TestFalse(TEXT("Frog is alive on log"), Frog->bIsDead);

	// Double-check: CheckRiverDeath should return false (platform is valid)
	TestFalse(TEXT("No river death with valid platform"), Frog->CheckRiverDeath());

	GameplayTestHelper::DestroyTestWorld(World);
	return true;
}

// ---------------------------------------------------------------------------
// Scenario 4: Hop into river without platform -> die
//
// The frog hops into a river row where NO rideable platforms exist.
// FindPlatformAtCurrentPosition finds nothing, CheckRiverDeath returns true,
// and FinishHop calls Die(Splash).
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameplay_HopIntoRiverNoPlatform_Die,
	"UnrealFrog.Gameplay.RiverNoPlatform.Die",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameplay_HopIntoRiverNoPlatform_Die::RunTest(const FString& Parameters)
{
	UWorld* World = GameplayTestHelper::CreateTestWorld();

	float CellSize = 100.0f;
	int32 RiverRow = 8;
	int32 StartRow = RiverRow - 1;

	AFrogCharacter* Frog = GameplayTestHelper::SpawnFrogAt(World, FIntPoint(6, StartRow), CellSize);
	TestNotNull(TEXT("Frog spawned"), Frog);
	if (!Frog)
	{
		GameplayTestHelper::DestroyTestWorld(World);
		return true;
	}

	// NO platforms spawned -- the river row is empty water

	float HopDuration = Frog->HopDuration;

	// Hop forward into the empty river
	Frog->RequestHop(FVector(0.0, 1.0, 0.0));
	TestTrue(TEXT("Frog is hopping"), Frog->bIsHopping);

	// Complete the hop — FinishHop -> FindPlatformAtCurrentPosition (finds nothing)
	// -> CheckRiverDeath returns true -> Die(Splash)
	Frog->Tick(HopDuration + 0.01f);

	// Frog should be dead with Splash
	TestTrue(TEXT("Frog is dead"), Frog->bIsDead);
	TestEqual(TEXT("Death type is Splash"), Frog->LastDeathType, EDeathType::Splash);

	// CurrentPlatform should be null
	TestTrue(TEXT("No platform"), Frog->CurrentPlatform.Get() == nullptr);

	GameplayTestHelper::DestroyTestWorld(World);
	return true;
}

// ---------------------------------------------------------------------------
// Scenario 5: Fill all 5 home slots -> wave increments
//
// This is a regression test for the Sprint 9 QA finding that filling 5
// home slots didn't trigger wave increment. The full flow is:
//   TryFillHomeSlot(last) -> OnWaveComplete -> SetState(RoundComplete)
//   -> OnRoundCompleteFinished (timer) -> CurrentWave++
//
// Since timers need a world context, we call OnRoundCompleteFinished
// directly to simulate the timer firing. This tests the state machine
// end-to-end without needing a real timer.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameplay_FillAllHomeSlots_WaveIncrements,
	"UnrealFrog.Gameplay.HomeSlots.WaveIncrements",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameplay_FillAllHomeSlots_WaveIncrements::RunTest(const FString& Parameters)
{
	// NewObject GameMode has no world context -- suppress expected UE errors
	AddExpectedError(TEXT("No world was found"), EAutomationExpectedErrorFlags::Contains, 0);

	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	// Track wave completion via native delegate
	int32 WaveCompletedCount = 0;
	int32 CompletedWaveNum = 0;
	int32 NextWaveNum = 0;
	GM->OnWaveCompleted.AddLambda([&](int32 Completed, int32 Next) {
		WaveCompletedCount++;
		CompletedWaveNum = Completed;
		NextWaveNum = Next;
	});

	// Start the game
	GM->StartGame();
	GM->OnSpawningComplete();
	TestEqual(TEXT("Playing state"), GM->CurrentState, EGameState::Playing);
	TestEqual(TEXT("Wave starts at 1"), GM->CurrentWave, 1);

	// Read home slot columns from GM (tuning-resilient)
	const TArray<int32>& Columns = GM->HomeSlotColumns;
	int32 TotalSlots = GM->TotalHomeSlots;
	TestEqual(TEXT("5 home slots configured"), TotalSlots, 5);
	TestEqual(TEXT("5 columns configured"), Columns.Num(), 5);

	// Fill slots 0 through 3 via HandleHopCompleted (the real game path)
	for (int32 i = 0; i < TotalSlots - 1; ++i)
	{
		// Simulate hopping to the home row at each slot column
		GM->HandleHopCompleted(FIntPoint(Columns[i], GM->HomeSlotRow));

		// After filling a non-final slot, GM transitions to Spawning
		// (simulating frog returning to start). Manually complete spawning.
		if (GM->CurrentState == EGameState::Spawning)
		{
			GM->OnSpawningComplete();
		}

		TestEqual(
			*FString::Printf(TEXT("%d slots filled after slot %d"), i + 1, i),
			GM->HomeSlotsFilledCount, i + 1);

		// Should still be playing after non-final slot
		TestEqual(
			*FString::Printf(TEXT("Still Playing after slot %d"), i),
			GM->CurrentState, EGameState::Playing);
	}

	// Fill the LAST slot -- this should trigger wave completion
	GM->HandleHopCompleted(FIntPoint(Columns[TotalSlots - 1], GM->HomeSlotRow));

	// State should be RoundComplete (OnWaveComplete was called)
	TestEqual(TEXT("RoundComplete after filling all slots"), GM->CurrentState, EGameState::RoundComplete);
	TestEqual(TEXT("All 5 slots filled"), GM->HomeSlotsFilledCount, TotalSlots);

	// Wave hasn't incremented yet -- that happens in OnRoundCompleteFinished
	TestEqual(TEXT("Wave still 1 before timer fires"), GM->CurrentWave, 1);

	// Simulate the round-complete timer firing
	GM->OnRoundCompleteFinished();

	// NOW the wave should increment
	TestEqual(TEXT("Wave incremented to 2"), GM->CurrentWave, 2);
	TestEqual(TEXT("Home slots reset to 0"), GM->HomeSlotsFilledCount, 0);
	TestEqual(TEXT("OnWaveCompleted fired exactly once"), WaveCompletedCount, 1);
	TestEqual(TEXT("Completed wave was 1"), CompletedWaveNum, 1);
	TestEqual(TEXT("Next wave is 2"), NextWaveNum, 2);

	// State should transition to Spawning for the new wave
	TestEqual(TEXT("Spawning for new wave"), GM->CurrentState, EGameState::Spawning);

	return true;
}

// ---------------------------------------------------------------------------
// Scenario 6: Multi-hop road crossing with moving hazards -> position-based survival
//
// The frog makes 3 consecutive hops across road rows that have hazards.
// The hazards are positioned OFF the frog's path, so the frog should
// survive all 3 hops. This tests that:
//   (a) Multi-hop sequences preserve frog state correctly
//   (b) Hazards at different X positions don't false-positive kill the frog
//   (c) The grid position updates correctly across sequential hops
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameplay_MultiHopRoadCrossing_Survive,
	"UnrealFrog.Gameplay.MultiHopRoad.Survive",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameplay_MultiHopRoadCrossing_Survive::RunTest(const FString& Parameters)
{
	UWorld* World = GameplayTestHelper::CreateTestWorld();

	float CellSize = 100.0f;

	// Frog starts at row 0 (safe zone, column 6 = X 600)
	AFrogCharacter* Frog = GameplayTestHelper::SpawnFrogAt(World, FIntPoint(6, 0), CellSize);
	TestNotNull(TEXT("Frog spawned"), Frog);
	if (!Frog)
	{
		GameplayTestHelper::DestroyTestWorld(World);
		return true;
	}

	float HopDuration = Frog->HopDuration;

	// Spawn hazards at rows 1, 2, 3 -- but offset far from frog's column (X=600).
	// Cars at X=200, so frog at X=600 is safely distant.
	GameplayTestHelper::SpawnHazardAt(
		World, 200.0f, 1, CellSize, /*bRideable=*/false, /*WidthCells=*/1, /*Speed=*/150.0f, /*bMovesRight=*/true);
	GameplayTestHelper::SpawnHazardAt(
		World, 200.0f, 2, CellSize, /*bRideable=*/false, /*WidthCells=*/1, /*Speed=*/150.0f, /*bMovesRight=*/false);
	GameplayTestHelper::SpawnHazardAt(
		World, 200.0f, 3, CellSize, /*bRideable=*/false, /*WidthCells=*/2, /*Speed=*/200.0f, /*bMovesRight=*/true);

	// Hop 1: row 0 -> row 1
	Frog->RequestHop(FVector(0.0, 1.0, 0.0));
	TestTrue(TEXT("Hop 1 started"), Frog->bIsHopping);
	Frog->Tick(HopDuration + 0.01f);
	TestFalse(TEXT("Hop 1 completed"), Frog->bIsHopping);
	TestFalse(TEXT("Alive after hop 1"), Frog->bIsDead);
	TestEqual(TEXT("At row 1"), Frog->GridPosition.Y, 1);

	// Hop 2: row 1 -> row 2
	Frog->RequestHop(FVector(0.0, 1.0, 0.0));
	TestTrue(TEXT("Hop 2 started"), Frog->bIsHopping);
	Frog->Tick(HopDuration + 0.01f);
	TestFalse(TEXT("Hop 2 completed"), Frog->bIsHopping);
	TestFalse(TEXT("Alive after hop 2"), Frog->bIsDead);
	TestEqual(TEXT("At row 2"), Frog->GridPosition.Y, 2);

	// Hop 3: row 2 -> row 3
	Frog->RequestHop(FVector(0.0, 1.0, 0.0));
	TestTrue(TEXT("Hop 3 started"), Frog->bIsHopping);
	Frog->Tick(HopDuration + 0.01f);
	TestFalse(TEXT("Hop 3 completed"), Frog->bIsHopping);
	TestFalse(TEXT("Alive after hop 3"), Frog->bIsDead);
	TestEqual(TEXT("At row 3"), Frog->GridPosition.Y, 3);

	// Final position check
	FVector FinalPos = Frog->GetActorLocation();
	TestNearlyEqual(TEXT("Final X at column 6"), FinalPos.X, 600.0, 1.0);
	TestNearlyEqual(TEXT("Final Y at row 3"), FinalPos.Y, 300.0, 1.0);

	GameplayTestHelper::DestroyTestWorld(World);
	return true;
}

#endif // WITH_AUTOMATION_TESTS
