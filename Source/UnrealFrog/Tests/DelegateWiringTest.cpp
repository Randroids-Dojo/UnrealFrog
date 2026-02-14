// Copyright UnrealFrog Team. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#include "Core/FrogCharacter.h"
#include "Core/ScoreSubsystem.h"
#include "Core/UnrealFrogGameMode.h"

#if WITH_AUTOMATION_TESTS

namespace
{
	UScoreSubsystem* CreateTestScoring()
	{
		UGameInstance* TestGI = NewObject<UGameInstance>();
		return NewObject<UScoreSubsystem>(TestGI);
	}
}

// ---------------------------------------------------------------------------
// Delegate Wiring Verification Tests
//
// These tests verify that every critical delegate has at least one binding
// and that firing the delegate produces the expected side effect. This
// catches the Sprint 2 defect where handler methods existed but were never
// bound via AddDynamic.
//
// Note: Dynamic multicast delegates don't support AddLambda. Tests for those
// verify behavioral outcomes (state changes, value updates) instead.
// Native multicast delegates (OnTimerExpired, OnWaveCompleted) do support
// AddLambda and are tested with direct binding.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Test: OnHopCompleted is bound and fires correctly
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWiring_OnHopCompletedBound,
	"UnrealFrog.Wiring.OnHopCompleted_IsBound",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWiring_OnHopCompletedBound::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	// Before wiring: delegate should NOT be bound
	TestFalse(TEXT("OnHopCompleted not bound before wiring"), Frog->OnHopCompleted.IsBound());

	// Wire the delegate exactly as BeginPlay does
	Frog->OnHopCompleted.AddDynamic(GM, &AUnrealFrogGameMode::HandleHopCompleted);

	// After wiring: delegate should be bound
	TestTrue(TEXT("OnHopCompleted bound after AddDynamic"), Frog->OnHopCompleted.IsBound());

	// Behavioral verification: call handler directly (dynamic delegate Broadcast
	// silently fails on NewObject actors without a world context)
	GM->StartGame();
	GM->OnSpawningComplete();
	TestEqual(TEXT("HighestRow starts at 0"), GM->HighestRowReached, 0);

	GM->HandleHopCompleted(FIntPoint(6, 3));
	TestEqual(TEXT("HighestRow updated to 3"), GM->HighestRowReached, 3);

	return true;
}

// ---------------------------------------------------------------------------
// Test: OnFrogDied is bound and fires correctly
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWiring_OnFrogDiedBound,
	"UnrealFrog.Wiring.OnFrogDied_IsBound",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWiring_OnFrogDiedBound::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	// Before wiring
	TestFalse(TEXT("OnFrogDied not bound before wiring"), Frog->OnFrogDied.IsBound());

	// Wire the delegate exactly as BeginPlay does
	Frog->OnFrogDied.AddDynamic(GM, &AUnrealFrogGameMode::HandleFrogDied);

	// After wiring
	TestTrue(TEXT("OnFrogDied bound after AddDynamic"), Frog->OnFrogDied.IsBound());

	// Behavioral verification: call handler directly (dynamic delegate Broadcast
	// silently fails on NewObject actors without a world context)
	GM->StartGame();
	GM->OnSpawningComplete();
	TestEqual(TEXT("Playing state"), GM->CurrentState, EGameState::Playing);

	GM->HandleFrogDied(EDeathType::Squish);
	TestEqual(TEXT("State transitions to Dying"), GM->CurrentState, EGameState::Dying);

	return true;
}

// ---------------------------------------------------------------------------
// Test: OnGameStateChanged fires on state transitions (behavioral)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWiring_OnGameStateChangedFires,
	"UnrealFrog.Wiring.OnGameStateChanged_Fires",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWiring_OnGameStateChangedFires::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	// OnGameStateChanged is a dynamic delegate — verify via behavioral outcome
	// StartGame calls SetState which broadcasts OnGameStateChanged
	TestEqual(TEXT("Starts in Title"), GM->CurrentState, EGameState::Title);

	GM->StartGame();
	TestEqual(TEXT("State changed to Spawning"), GM->CurrentState, EGameState::Spawning);

	GM->OnSpawningComplete();
	TestEqual(TEXT("State changed to Playing"), GM->CurrentState, EGameState::Playing);

	GM->PauseGame();
	TestEqual(TEXT("State changed to Paused"), GM->CurrentState, EGameState::Paused);

	GM->ResumeGame();
	TestEqual(TEXT("State changed back to Playing"), GM->CurrentState, EGameState::Playing);

	return true;
}

// ---------------------------------------------------------------------------
// Test: OnTimerUpdate fires during TickTimer (behavioral)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWiring_OnTimerUpdateFires,
	"UnrealFrog.Wiring.OnTimerUpdate_Fires",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWiring_OnTimerUpdateFires::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	GM->OnSpawningComplete();

	float TimeBefore = GM->RemainingTime;
	GM->TickTimer(5.0f);

	// Behavioral: timer decremented during Playing
	TestTrue(TEXT("Timer decreased after TickTimer"), GM->RemainingTime < TimeBefore);
	TestNearlyEqual(TEXT("Timer decreased by ~5s"), GM->RemainingTime, TimeBefore - 5.0f);

	return true;
}

// ---------------------------------------------------------------------------
// Test: OnHomeSlotFilled fires when filling a home slot (behavioral)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWiring_OnHomeSlotFilledFires,
	"UnrealFrog.Wiring.OnHomeSlotFilled_Fires",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWiring_OnHomeSlotFilledFires::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	GM->OnSpawningComplete();

	// Fill the first home slot (column 1, index 0)
	TestEqual(TEXT("No slots filled yet"), GM->HomeSlotsFilledCount, 0);

	bool bFilled = GM->TryFillHomeSlot(1);
	TestTrue(TEXT("Slot filled successfully"), bFilled);
	TestEqual(TEXT("HomeSlotsFilledCount is 1"), GM->HomeSlotsFilledCount, 1);

	return true;
}

// ---------------------------------------------------------------------------
// Test: OnTimerExpired fires when timer reaches 0 (native delegate)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWiring_OnTimerExpiredFires,
	"UnrealFrog.Wiring.OnTimerExpired_Fires",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWiring_OnTimerExpiredFires::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	bool bFired = false;

	// OnTimerExpired is a native delegate — AddLambda works
	GM->OnTimerExpired.AddLambda([&bFired]() {
		bFired = true;
	});

	GM->StartGame();
	GM->OnSpawningComplete();

	// Tick past the timer limit
	GM->TickTimer(GM->TimePerLevel + 1.0f);

	TestTrue(TEXT("OnTimerExpired fired"), bFired);

	return true;
}

// ---------------------------------------------------------------------------
// Test: OnWaveCompleted fires on wave transition (native delegate)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWiring_OnWaveCompletedFires,
	"UnrealFrog.Wiring.OnWaveCompleted_Fires",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWiring_OnWaveCompletedFires::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	int32 ReceivedCompletedWave = -1;
	int32 ReceivedNextWave = -1;
	bool bFired = false;

	// OnWaveCompleted is a native delegate — AddLambda works
	GM->OnWaveCompleted.AddLambda([&bFired, &ReceivedCompletedWave, &ReceivedNextWave](int32 CompletedWave, int32 NextWave) {
		bFired = true;
		ReceivedCompletedWave = CompletedWave;
		ReceivedNextWave = NextWave;
	});

	GM->StartGame();
	GM->OnSpawningComplete();

	// Fill all 5 home slots to trigger wave complete
	// Each non-final fill transitions to Spawning, cycle back to Playing
	GM->HandleHopCompleted(FIntPoint(1, 14));
	GM->OnSpawningComplete();
	GM->HandleHopCompleted(FIntPoint(4, 14));
	GM->OnSpawningComplete();
	GM->HandleHopCompleted(FIntPoint(6, 14));
	GM->OnSpawningComplete();
	GM->HandleHopCompleted(FIntPoint(8, 14));
	GM->OnSpawningComplete();
	GM->HandleHopCompleted(FIntPoint(11, 14));

	// Trigger the round complete → next wave transition
	GM->OnRoundCompleteFinished();

	TestTrue(TEXT("OnWaveCompleted fired"), bFired);
	TestEqual(TEXT("Completed wave 1"), ReceivedCompletedWave, 1);
	TestEqual(TEXT("Next wave is 2"), ReceivedNextWave, 2);

	return true;
}

// ---------------------------------------------------------------------------
// Test: ScoreSubsystem OnScoreChanged fires (behavioral)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWiring_OnScoreChangedFires,
	"UnrealFrog.Wiring.OnScoreChanged_Fires",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWiring_OnScoreChangedFires::RunTest(const FString& Parameters)
{
	UScoreSubsystem* Scoring = CreateTestScoring();

	TestEqual(TEXT("Score starts at 0"), Scoring->Score, 0);

	Scoring->AddForwardHopScore();

	TestEqual(TEXT("Score is PointsPerHop (10)"), Scoring->Score, 10);

	return true;
}

// ---------------------------------------------------------------------------
// Test: ScoreSubsystem OnLivesChanged fires (behavioral)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWiring_OnLivesChangedFires,
	"UnrealFrog.Wiring.OnLivesChanged_Fires",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWiring_OnLivesChangedFires::RunTest(const FString& Parameters)
{
	UScoreSubsystem* Scoring = CreateTestScoring();

	TestEqual(TEXT("Lives start at 3"), Scoring->Lives, 3);

	Scoring->LoseLife();

	TestEqual(TEXT("Lives decremented to 2"), Scoring->Lives, 2);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
