// Copyright UnrealFrog Team. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Core/UnrealFrogGameMode.h"

#if WITH_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Test: StartGame transitions to Spawning (not directly to Playing)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FOrchestration_StartGoesToSpawning,
	"UnrealFrog.Orchestration.StartGoesToSpawning",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FOrchestration_StartGoesToSpawning::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	TestEqual(TEXT("Starts in Title"), GM->CurrentState, EGameState::Title);

	GM->StartGame();

	TestEqual(TEXT("After StartGame, state is Spawning"), GM->CurrentState, EGameState::Spawning);

	return true;
}

// ---------------------------------------------------------------------------
// Test: OnSpawningComplete transitions from Spawning to Playing
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FOrchestration_SpawningToPlaying,
	"UnrealFrog.Orchestration.SpawningToPlaying",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FOrchestration_SpawningToPlaying::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	TestEqual(TEXT("In Spawning"), GM->CurrentState, EGameState::Spawning);

	GM->OnSpawningComplete();
	TestEqual(TEXT("Now Playing"), GM->CurrentState, EGameState::Playing);

	return true;
}

// ---------------------------------------------------------------------------
// Test: HandleFrogDied transitions from Playing to Dying
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FOrchestration_DiedGoesToDying,
	"UnrealFrog.Orchestration.DiedGoesToDying",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FOrchestration_DiedGoesToDying::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	GM->OnSpawningComplete();
	TestEqual(TEXT("In Playing"), GM->CurrentState, EGameState::Playing);

	GM->HandleFrogDied(EDeathType::Squish);
	TestEqual(TEXT("Now Dying"), GM->CurrentState, EGameState::Dying);

	return true;
}

// ---------------------------------------------------------------------------
// Test: HandleFrogDied ignored when not in Playing state
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FOrchestration_DiedIgnoredOutsidePlaying,
	"UnrealFrog.Orchestration.DiedIgnoredOutsidePlaying",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FOrchestration_DiedIgnoredOutsidePlaying::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	// Title state — should ignore
	GM->HandleFrogDied(EDeathType::Squish);
	TestEqual(TEXT("Still Title"), GM->CurrentState, EGameState::Title);

	// Spawning state — should ignore
	GM->StartGame();
	GM->HandleFrogDied(EDeathType::Squish);
	TestEqual(TEXT("Still Spawning"), GM->CurrentState, EGameState::Spawning);

	return true;
}

// ---------------------------------------------------------------------------
// Test: OnDyingComplete without game over → Spawning
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FOrchestration_DyingToSpawning,
	"UnrealFrog.Orchestration.DyingToSpawning",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FOrchestration_DyingToSpawning::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	GM->OnSpawningComplete();
	GM->HandleFrogDied(EDeathType::Squish);
	TestEqual(TEXT("In Dying"), GM->CurrentState, EGameState::Dying);

	// bPendingGameOver defaults to false
	GM->OnDyingComplete();
	TestEqual(TEXT("Back to Spawning"), GM->CurrentState, EGameState::Spawning);

	return true;
}

// ---------------------------------------------------------------------------
// Test: OnDyingComplete with game over → GameOver state
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FOrchestration_DyingToGameOver,
	"UnrealFrog.Orchestration.DyingToGameOver",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FOrchestration_DyingToGameOver::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	GM->OnSpawningComplete();
	GM->HandleFrogDied(EDeathType::Squish);

	GM->bPendingGameOver = true;
	GM->OnDyingComplete();
	TestEqual(TEXT("GameOver after last life"), GM->CurrentState, EGameState::GameOver);

	return true;
}

// ---------------------------------------------------------------------------
// Test: HandleHopCompleted tracks highest row for scoring
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FOrchestration_ForwardHopTracking,
	"UnrealFrog.Orchestration.ForwardHopTracking",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FOrchestration_ForwardHopTracking::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	GM->OnSpawningComplete();

	TestEqual(TEXT("HighestRowReached starts at 0"), GM->HighestRowReached, 0);

	// Forward hop to row 1
	GM->HandleHopCompleted(FIntPoint(6, 1));
	TestEqual(TEXT("HighestRowReached updated to 1"), GM->HighestRowReached, 1);

	// Forward hop to row 2
	GM->HandleHopCompleted(FIntPoint(6, 2));
	TestEqual(TEXT("HighestRowReached updated to 2"), GM->HighestRowReached, 2);

	// Backward hop to row 1 — highest should not decrease
	GM->HandleHopCompleted(FIntPoint(6, 1));
	TestEqual(TEXT("HighestRowReached stays at 2"), GM->HighestRowReached, 2);

	return true;
}

// ---------------------------------------------------------------------------
// Test: HandleHopCompleted on home slot row fills slot
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FOrchestration_HomeSlotFill,
	"UnrealFrog.Orchestration.HomeSlotFill",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FOrchestration_HomeSlotFill::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	GM->OnSpawningComplete();

	TestEqual(TEXT("No home slots filled"), GM->HomeSlotsFilledCount, 0);

	// Hop to home slot row at a valid home slot column (column 4)
	GM->HandleHopCompleted(FIntPoint(4, 14));
	TestEqual(TEXT("One home slot filled"), GM->HomeSlotsFilledCount, 1);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Wave completes when all 5 home slots filled → RoundComplete state
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FOrchestration_AllHomeSlotsFilled,
	"UnrealFrog.Orchestration.AllHomeSlotsFilled",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FOrchestration_AllHomeSlotsFilled::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	GM->OnSpawningComplete();

	// Fill all 5 home slot columns: 1, 4, 6, 8, 11
	// Each non-final fill transitions to Spawning (frog respawn), so cycle back to Playing
	GM->HandleHopCompleted(FIntPoint(1, 14));
	TestEqual(TEXT("Spawning after slot 1"), GM->CurrentState, EGameState::Spawning);
	GM->OnSpawningComplete();

	GM->HandleHopCompleted(FIntPoint(4, 14));
	GM->OnSpawningComplete();

	GM->HandleHopCompleted(FIntPoint(6, 14));
	GM->OnSpawningComplete();

	GM->HandleHopCompleted(FIntPoint(8, 14));
	TestEqual(TEXT("4 slots filled"), GM->HomeSlotsFilledCount, 4);
	GM->OnSpawningComplete();

	GM->HandleHopCompleted(FIntPoint(11, 14));
	TestEqual(TEXT("State is RoundComplete"), GM->CurrentState, EGameState::RoundComplete);

	return true;
}

// ---------------------------------------------------------------------------
// Test: OnRoundCompleteFinished increments wave and goes to Spawning
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FOrchestration_RoundCompleteToSpawning,
	"UnrealFrog.Orchestration.RoundCompleteToSpawning",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FOrchestration_RoundCompleteToSpawning::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	GM->OnSpawningComplete();

	int32 InitialWave = GM->CurrentWave;

	// Fill all home slots to trigger round complete
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

	TestEqual(TEXT("State is RoundComplete"), GM->CurrentState, EGameState::RoundComplete);

	GM->OnRoundCompleteFinished();
	TestEqual(TEXT("Wave incremented"), GM->CurrentWave, InitialWave + 1);
	TestEqual(TEXT("State is Spawning"), GM->CurrentState, EGameState::Spawning);
	TestEqual(TEXT("Home slots reset"), GM->HomeSlotsFilledCount, 0);
	TestEqual(TEXT("HighestRow reset"), GM->HighestRowReached, 0);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Timer expiry triggers death
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FOrchestration_TimerExpiryKills,
	"UnrealFrog.Orchestration.TimerExpiryKills",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FOrchestration_TimerExpiryKills::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	GM->OnSpawningComplete();
	TestEqual(TEXT("Playing"), GM->CurrentState, EGameState::Playing);

	// Tick the timer past expiry
	GM->TickTimer(GM->TimePerLevel + 1.0f);
	TestEqual(TEXT("Timer expired → Dying"), GM->CurrentState, EGameState::Dying);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
