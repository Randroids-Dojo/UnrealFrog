// Copyright UnrealFrog Team. All Rights Reserved.
//
// [WaveCompletion] Regression tests for wave completion state machine.
//
// Sprint 9 QA reported that filling all 5 home slots did not trigger
// wave increment. These tests exercise the full flow:
//   TryFillHomeSlot -> OnWaveComplete -> RoundComplete -> OnRoundCompleteFinished -> Wave++
//
// Sprint 13, Task P0-2 -- Engine Architect
//
// Agreement Section 2: Tuning-resilient -- read HomeSlotColumns, TotalHomeSlots,
// and HomeSlotRow from the GameMode at test time, not hardcoded.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Core/UnrealFrogGameMode.h"

#if WITH_AUTOMATION_TESTS

// Helper: create a GM in Playing state, ready for home slot tests.
// Returns nullptr on failure (caller must handle).
static AUnrealFrogGameMode* CreatePlayingGameMode()
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();
	GM->StartGame();
	GM->OnSpawningComplete();
	return GM;
}

// ---------------------------------------------------------------------------
// Test 1: CurrentWave starts at 1 after StartGame
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaveCompletion_InitialWaveIsOne,
	"UnrealFrog.WaveCompletion.InitialWaveIsOne",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWaveCompletion_InitialWaveIsOne::RunTest(const FString& Parameters)
{
	AddExpectedError(TEXT("No world was found"), EAutomationExpectedErrorFlags::Contains, 0);

	AUnrealFrogGameMode* GM = CreatePlayingGameMode();

	TestEqual(TEXT("CurrentWave starts at 1"), GM->CurrentWave, 1);
	TestEqual(TEXT("State is Playing"), GM->CurrentState, EGameState::Playing);
	TestEqual(TEXT("HomeSlotsFilledCount is 0"), GM->HomeSlotsFilledCount, 0);

	return true;
}

// ---------------------------------------------------------------------------
// Test 2: TryFillHomeSlot returns true for each valid column
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaveCompletion_TryFillValidColumns,
	"UnrealFrog.WaveCompletion.TryFillValidColumns",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWaveCompletion_TryFillValidColumns::RunTest(const FString& Parameters)
{
	AddExpectedError(TEXT("No world was found"), EAutomationExpectedErrorFlags::Contains, 0);

	AUnrealFrogGameMode* GM = CreatePlayingGameMode();
	const TArray<int32>& Columns = GM->HomeSlotColumns;

	for (int32 i = 0; i < Columns.Num(); ++i)
	{
		bool bFilled = GM->TryFillHomeSlot(Columns[i]);
		TestTrue(
			*FString::Printf(TEXT("TryFillHomeSlot(%d) returns true"), Columns[i]),
			bFilled);
		TestEqual(
			*FString::Printf(TEXT("HomeSlotsFilledCount is %d after filling column %d"), i + 1, Columns[i]),
			GM->HomeSlotsFilledCount, i + 1);
	}

	return true;
}

// ---------------------------------------------------------------------------
// Test 3: TryFillHomeSlot returns false for invalid (non-slot) column
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaveCompletion_TryFillInvalidColumn,
	"UnrealFrog.WaveCompletion.TryFillInvalidColumn",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWaveCompletion_TryFillInvalidColumn::RunTest(const FString& Parameters)
{
	AddExpectedError(TEXT("No world was found"), EAutomationExpectedErrorFlags::Contains, 0);

	AUnrealFrogGameMode* GM = CreatePlayingGameMode();

	// Column 3 is not a home slot column (valid columns are 1, 4, 6, 8, 11)
	bool bFilled = GM->TryFillHomeSlot(3);
	TestFalse(TEXT("TryFillHomeSlot(3) returns false for non-slot column"), bFilled);
	TestEqual(TEXT("HomeSlotsFilledCount unchanged"), GM->HomeSlotsFilledCount, 0);

	return true;
}

// ---------------------------------------------------------------------------
// Test 4: TryFillHomeSlot returns false for already-filled slot
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaveCompletion_TryFillAlreadyFilledSlot,
	"UnrealFrog.WaveCompletion.TryFillAlreadyFilledSlot",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWaveCompletion_TryFillAlreadyFilledSlot::RunTest(const FString& Parameters)
{
	AddExpectedError(TEXT("No world was found"), EAutomationExpectedErrorFlags::Contains, 0);

	AUnrealFrogGameMode* GM = CreatePlayingGameMode();
	const TArray<int32>& Columns = GM->HomeSlotColumns;

	// Fill the first slot
	bool bFirst = GM->TryFillHomeSlot(Columns[0]);
	TestTrue(TEXT("First fill succeeds"), bFirst);
	TestEqual(TEXT("1 slot filled"), GM->HomeSlotsFilledCount, 1);

	// Try to fill the same slot again
	bool bSecond = GM->TryFillHomeSlot(Columns[0]);
	TestFalse(TEXT("Second fill of same slot returns false"), bSecond);
	TestEqual(TEXT("Still 1 slot filled"), GM->HomeSlotsFilledCount, 1);

	return true;
}

// ---------------------------------------------------------------------------
// Test 5: Filling all 5 slots transitions state to RoundComplete
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaveCompletion_AllSlotsFilled_RoundComplete,
	"UnrealFrog.WaveCompletion.AllSlotsFilled.RoundComplete",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWaveCompletion_AllSlotsFilled_RoundComplete::RunTest(const FString& Parameters)
{
	AddExpectedError(TEXT("No world was found"), EAutomationExpectedErrorFlags::Contains, 0);

	AUnrealFrogGameMode* GM = CreatePlayingGameMode();
	const TArray<int32>& Columns = GM->HomeSlotColumns;
	int32 TotalSlots = GM->TotalHomeSlots;

	TestEqual(TEXT("5 home slots configured"), TotalSlots, 5);

	// Fill all slots via TryFillHomeSlot (direct API, no HandleHopCompleted)
	for (int32 i = 0; i < TotalSlots; ++i)
	{
		GM->TryFillHomeSlot(Columns[i]);
	}

	TestEqual(TEXT("All 5 slots filled"), GM->HomeSlotsFilledCount, TotalSlots);
	TestEqual(TEXT("State is RoundComplete"), GM->CurrentState, EGameState::RoundComplete);
	// Wave has NOT incremented yet -- that happens in OnRoundCompleteFinished
	TestEqual(TEXT("Wave still 1 before timer fires"), GM->CurrentWave, 1);

	return true;
}

// ---------------------------------------------------------------------------
// Test 6: OnRoundCompleteFinished increments wave and resets slots
//
// This is the core regression test for the Sprint 9 QA finding. The full
// flow is exercised: fill all slots -> RoundComplete -> OnRoundCompleteFinished
// -> wave increments to 2, home slots reset, state transitions to Spawning.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaveCompletion_RoundCompleteFinished_WaveIncrements,
	"UnrealFrog.WaveCompletion.RoundCompleteFinished.WaveIncrements",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWaveCompletion_RoundCompleteFinished_WaveIncrements::RunTest(const FString& Parameters)
{
	AddExpectedError(TEXT("No world was found"), EAutomationExpectedErrorFlags::Contains, 0);

	AUnrealFrogGameMode* GM = CreatePlayingGameMode();
	const TArray<int32>& Columns = GM->HomeSlotColumns;

	// Track wave completion via native delegate (supports AddLambda)
	int32 WaveCompletedCount = 0;
	int32 CompletedWaveNum = 0;
	int32 NextWaveNum = 0;
	GM->OnWaveCompleted.AddLambda([&](int32 Completed, int32 Next) {
		WaveCompletedCount++;
		CompletedWaveNum = Completed;
		NextWaveNum = Next;
	});

	// Fill all home slots
	for (int32 i = 0; i < GM->TotalHomeSlots; ++i)
	{
		GM->TryFillHomeSlot(Columns[i]);
	}

	TestEqual(TEXT("State is RoundComplete"), GM->CurrentState, EGameState::RoundComplete);
	TestEqual(TEXT("Wave still 1"), GM->CurrentWave, 1);

	// Simulate the round-complete timer firing
	GM->OnRoundCompleteFinished();

	// Verify the wave incremented
	TestEqual(TEXT("CurrentWave incremented to 2"), GM->CurrentWave, 2);

	// Verify home slots were reset
	TestEqual(TEXT("HomeSlotsFilledCount reset to 0"), GM->HomeSlotsFilledCount, 0);
	for (int32 i = 0; i < GM->HomeSlots.Num(); ++i)
	{
		TestFalse(
			*FString::Printf(TEXT("HomeSlot[%d] is unfilled"), i),
			GM->HomeSlots[i]);
	}

	// Verify delegate fired with correct values
	TestEqual(TEXT("OnWaveCompleted fired once"), WaveCompletedCount, 1);
	TestEqual(TEXT("CompletedWave was 1"), CompletedWaveNum, 1);
	TestEqual(TEXT("NextWave is 2"), NextWaveNum, 2);

	// State should be Spawning for the new wave
	TestEqual(TEXT("State is Spawning for new wave"), GM->CurrentState, EGameState::Spawning);

	// Timer and tracking state should be reset
	TestEqual(TEXT("RemainingTime reset to TimePerLevel"), GM->RemainingTime, GM->TimePerLevel);
	TestEqual(TEXT("HighestRowReached reset"), GM->HighestRowReached, 0);
	TestFalse(TEXT("bTimerWarningPlayed reset"), GM->bTimerWarningPlayed);

	return true;
}

// ---------------------------------------------------------------------------
// Test 7: Full wave cycle via HandleHopCompleted (game path)
//
// Exercises the real game path: frog hops to HomeSlotRow at each column.
// After filling non-final slots, GM transitions to Spawning then Playing.
// After the final slot, GM goes to RoundComplete.
// After OnRoundCompleteFinished, wave 2 begins.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaveCompletion_FullCycleViaHandleHopCompleted,
	"UnrealFrog.WaveCompletion.FullCycle.ViaHandleHopCompleted",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWaveCompletion_FullCycleViaHandleHopCompleted::RunTest(const FString& Parameters)
{
	AddExpectedError(TEXT("No world was found"), EAutomationExpectedErrorFlags::Contains, 0);

	AUnrealFrogGameMode* GM = CreatePlayingGameMode();
	const TArray<int32>& Columns = GM->HomeSlotColumns;
	int32 TotalSlots = GM->TotalHomeSlots;
	int32 HomeRow = GM->HomeSlotRow;

	// Fill slots 0 through N-2 via HandleHopCompleted
	for (int32 i = 0; i < TotalSlots - 1; ++i)
	{
		GM->HandleHopCompleted(FIntPoint(Columns[i], HomeRow));

		// After filling a non-final slot, GM transitions to Spawning
		if (GM->CurrentState == EGameState::Spawning)
		{
			GM->OnSpawningComplete();
		}

		TestEqual(
			*FString::Printf(TEXT("%d slot(s) filled"), i + 1),
			GM->HomeSlotsFilledCount, i + 1);
		TestEqual(TEXT("Still Playing"), GM->CurrentState, EGameState::Playing);
	}

	// Fill the last slot -- triggers wave completion
	GM->HandleHopCompleted(FIntPoint(Columns[TotalSlots - 1], HomeRow));
	TestEqual(TEXT("RoundComplete after last slot"), GM->CurrentState, EGameState::RoundComplete);
	TestEqual(TEXT("All slots filled"), GM->HomeSlotsFilledCount, TotalSlots);
	TestEqual(TEXT("Wave still 1 before OnRoundCompleteFinished"), GM->CurrentWave, 1);

	// Simulate timer firing
	GM->OnRoundCompleteFinished();

	TestEqual(TEXT("Wave 2 after round complete"), GM->CurrentWave, 2);
	TestEqual(TEXT("Slots reset to 0"), GM->HomeSlotsFilledCount, 0);
	TestEqual(TEXT("Spawning state for wave 2"), GM->CurrentState, EGameState::Spawning);

	return true;
}

// ---------------------------------------------------------------------------
// Test 8: OnRoundCompleteFinished is a no-op if state is not RoundComplete
//
// Guards against accidental double-firing of the round-complete timer.
// If state has already transitioned away from RoundComplete (e.g., by a
// spurious timer event), the function must early-return without side effects.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaveCompletion_RoundCompleteFinished_GuardedByState,
	"UnrealFrog.WaveCompletion.RoundCompleteFinished.GuardedByState",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWaveCompletion_RoundCompleteFinished_GuardedByState::RunTest(const FString& Parameters)
{
	AddExpectedError(TEXT("No world was found"), EAutomationExpectedErrorFlags::Contains, 0);

	AUnrealFrogGameMode* GM = CreatePlayingGameMode();

	// State is Playing, not RoundComplete. Calling OnRoundCompleteFinished
	// should be a no-op.
	int32 WaveBefore = GM->CurrentWave;
	GM->OnRoundCompleteFinished();

	TestEqual(TEXT("Wave unchanged when not in RoundComplete"), GM->CurrentWave, WaveBefore);
	TestEqual(TEXT("State unchanged"), GM->CurrentState, EGameState::Playing);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
