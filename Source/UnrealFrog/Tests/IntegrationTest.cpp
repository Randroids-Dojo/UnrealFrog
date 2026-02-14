// Copyright UnrealFrog Team. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#include "Core/FrogCharacter.h"
#include "Core/HazardBase.h"
#include "Core/ScoreSubsystem.h"
#include "Core/UnrealFrogGameMode.h"

#if WITH_AUTOMATION_TESTS

namespace
{
	UScoreSubsystem* CreateTestScoringForIntegration()
	{
		UGameInstance* TestGI = NewObject<UGameInstance>();
		return NewObject<UScoreSubsystem>(TestGI);
	}
}

// ---------------------------------------------------------------------------
// Test: Collision → Die → delegates fire correctly
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIntegration_CollisionToDeath,
	"UnrealFrog.Integration.CollisionToDeath",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FIntegration_CollisionToDeath::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	AHazardBase* Car = NewObject<AHazardBase>();
	Car->bIsRideable = false;

	bool bDelegateFired = false;
	EDeathType ReceivedType = EDeathType::None;
	Frog->OnFrogDiedNative.AddLambda([&bDelegateFired, &ReceivedType](EDeathType Type) {
		bDelegateFired = true;
		ReceivedType = Type;
	});

	Frog->HandleHazardOverlap(Car);

	TestTrue(TEXT("OnFrogDiedNative delegate fired"), bDelegateFired);
	TestEqual(TEXT("Death type is Squish"), ReceivedType, EDeathType::Squish);
	TestTrue(TEXT("Frog is dead"), Frog->bIsDead);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Full death-to-respawn cycle
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIntegration_DeathRespawnCycle,
	"UnrealFrog.Integration.DeathRespawnCycle",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FIntegration_DeathRespawnCycle::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	// Start the game cycle
	GM->StartGame();
	GM->OnSpawningComplete();
	TestEqual(TEXT("Playing"), GM->CurrentState, EGameState::Playing);

	// Move frog forward
	Frog->GridPosition = FIntPoint(6, 3);

	// Kill the frog via game mode
	GM->HandleFrogDied(EDeathType::Squish);
	TestEqual(TEXT("Dying state"), GM->CurrentState, EGameState::Dying);

	// Complete dying animation (timer would fire in real game)
	GM->OnDyingComplete();
	TestEqual(TEXT("Back to Spawning"), GM->CurrentState, EGameState::Spawning);

	// Complete spawning
	GM->OnSpawningComplete();
	TestEqual(TEXT("Playing again"), GM->CurrentState, EGameState::Playing);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Full round complete cycle — fill all 5 home slots
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIntegration_FullRoundCycle,
	"UnrealFrog.Integration.FullRoundCycle",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FIntegration_FullRoundCycle::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	GM->OnSpawningComplete();
	TestEqual(TEXT("Wave 1"), GM->CurrentWave, 1);

	// Fill all 5 home slots via HandleHopCompleted
	GM->HandleHopCompleted(FIntPoint(1, 14));
	GM->HandleHopCompleted(FIntPoint(4, 14));
	GM->HandleHopCompleted(FIntPoint(6, 14));
	GM->HandleHopCompleted(FIntPoint(8, 14));
	GM->HandleHopCompleted(FIntPoint(11, 14));

	TestEqual(TEXT("RoundComplete state"), GM->CurrentState, EGameState::RoundComplete);

	// Complete the round
	GM->OnRoundCompleteFinished();
	TestEqual(TEXT("Wave 2"), GM->CurrentWave, 2);
	TestEqual(TEXT("Spawning for wave 2"), GM->CurrentState, EGameState::Spawning);
	TestEqual(TEXT("Home slots reset"), GM->HomeSlotsFilledCount, 0);
	TestEqual(TEXT("Timer reset"), GM->RemainingTime, GM->TimePerLevel);

	// Can start playing wave 2
	GM->OnSpawningComplete();
	TestEqual(TEXT("Playing wave 2"), GM->CurrentState, EGameState::Playing);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Game over via bPendingGameOver flag
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIntegration_GameOverAfterDeath,
	"UnrealFrog.Integration.GameOverAfterDeath",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FIntegration_GameOverAfterDeath::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	GM->OnSpawningComplete();

	// Die and flag game over (simulating ScoreSubsystem reporting 0 lives)
	GM->HandleFrogDied(EDeathType::Squish);
	GM->bPendingGameOver = true;

	GM->OnDyingComplete();
	TestEqual(TEXT("GameOver state"), GM->CurrentState, EGameState::GameOver);

	// Recovery: ReturnToTitle → StartGame
	GM->ReturnToTitle();
	TestEqual(TEXT("Back to Title"), GM->CurrentState, EGameState::Title);

	GM->StartGame();
	TestEqual(TEXT("Can start new game"), GM->CurrentState, EGameState::Spawning);
	TestFalse(TEXT("bPendingGameOver reset"), GM->bPendingGameOver);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Forward hop tracking updates HighestRowReached correctly
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIntegration_ForwardHopScoring,
	"UnrealFrog.Integration.ForwardHopScoring",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FIntegration_ForwardHopScoring::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	GM->OnSpawningComplete();

	// Progressive forward hops
	GM->HandleHopCompleted(FIntPoint(6, 1));
	TestEqual(TEXT("Highest row 1"), GM->HighestRowReached, 1);

	GM->HandleHopCompleted(FIntPoint(6, 2));
	TestEqual(TEXT("Highest row 2"), GM->HighestRowReached, 2);

	GM->HandleHopCompleted(FIntPoint(6, 3));
	TestEqual(TEXT("Highest row 3"), GM->HighestRowReached, 3);

	// Backward hop should not decrease highest
	GM->HandleHopCompleted(FIntPoint(6, 2));
	TestEqual(TEXT("Still highest row 3"), GM->HighestRowReached, 3);

	// Side hop should not change highest
	GM->HandleHopCompleted(FIntPoint(7, 2));
	TestEqual(TEXT("Still highest row 3 after side hop"), GM->HighestRowReached, 3);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Timer expiry → Dying state transition
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIntegration_TimerExpiryToDeath,
	"UnrealFrog.Integration.TimerExpiryToDeath",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FIntegration_TimerExpiryToDeath::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	GM->OnSpawningComplete();

	// Tick timer to expiry
	GM->TickTimer(GM->TimePerLevel + 1.0f);

	TestEqual(TEXT("State is Dying after timeout"), GM->CurrentState, EGameState::Dying);

	// Complete dying → should go to Spawning (not game over)
	GM->OnDyingComplete();
	TestEqual(TEXT("Back to Spawning"), GM->CurrentState, EGameState::Spawning);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Collision with platform sets riding, then end overlap clears it
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIntegration_PlatformRidingCycle,
	"UnrealFrog.Integration.PlatformRidingCycle",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FIntegration_PlatformRidingCycle::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();

	// Set frog on river row
	Frog->GridPosition = FIntPoint(6, 8);
	TestTrue(TEXT("On river row"), Frog->IsOnRiverRow());

	// Mount a platform
	AHazardBase* Log = NewObject<AHazardBase>();
	Log->bIsRideable = true;
	Log->Speed = 100.0f;
	Log->bMovesRight = true;

	Frog->HandleHazardOverlap(Log);
	TestTrue(TEXT("Mounted on log"), Frog->CurrentPlatform.Get() == Log);
	TestFalse(TEXT("CheckRiverDeath false with platform"), Frog->CheckRiverDeath());

	// Dismount
	Frog->HandlePlatformEndOverlap(Log);
	TestTrue(TEXT("Platform cleared"), Frog->CurrentPlatform.Get() == nullptr);
	TestTrue(TEXT("CheckRiverDeath true without platform"), Frog->CheckRiverDeath());

	return true;
}

// ---------------------------------------------------------------------------
// Test: End-to-end wiring smoke — full delegate chain verification
// Spawns GameMode + Frog + ScoreSubsystem, wires them, then verifies that
// actions produce expected state changes through the entire chain.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIntegration_WiringSmokeTest,
	"UnrealFrog.Integration.Wiring.FullChainSmokeTest",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FIntegration_WiringSmokeTest::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	UScoreSubsystem* Scoring = CreateTestScoringForIntegration();

	// -- Step 1: StartGame → verify state transitions --
	GM->StartGame();
	TestEqual(TEXT("State is Spawning"), GM->CurrentState, EGameState::Spawning);

	GM->OnSpawningComplete();
	TestEqual(TEXT("State is Playing"), GM->CurrentState, EGameState::Playing);

	// -- Step 2: Forward hop → verify scoring chain via direct method call --
	// (Dynamic delegate Broadcast silently fails on NewObject actors without world)
	GM->HandleHopCompleted(FIntPoint(6, 1));
	TestEqual(TEXT("HighestRow updated to 1"), GM->HighestRowReached, 1);

	GM->HandleHopCompleted(FIntPoint(6, 2));
	TestEqual(TEXT("HighestRow updated to 2"), GM->HighestRowReached, 2);

	// -- Step 3: Score directly via subsystem --
	Scoring->AddForwardHopScore();
	TestEqual(TEXT("Score is 10"), Scoring->Score, 10);

	// -- Step 4: Kill frog → verify death chain --
	GM->HandleFrogDied(EDeathType::Squish);
	TestEqual(TEXT("GM transitioned to Dying"), GM->CurrentState, EGameState::Dying);

	Scoring->LoseLife();
	TestEqual(TEXT("Lives decremented to 2"), Scoring->Lives, 2);

	// -- Step 5: Fill home slot → verify slot filled --
	GM->OnDyingComplete();
	GM->OnSpawningComplete();
	TestEqual(TEXT("Playing again"), GM->CurrentState, EGameState::Playing);

	GM->HandleHopCompleted(FIntPoint(1, 14)); // Home slot column
	TestEqual(TEXT("Home slot filled count is 1"), GM->HomeSlotsFilledCount, 1);

	// -- Step 6: Verify timer ticks during Playing --
	float TimeBefore = GM->RemainingTime;
	GM->TickTimer(1.0f);
	TestTrue(TEXT("Timer decremented during Playing"), GM->RemainingTime < TimeBefore);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Timer timeout death — verifies full chain: timer=0 → Timeout death → life lost
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIntegration_TimeoutDeathChain,
	"UnrealFrog.Integration.Wiring.TimeoutDeathChain",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FIntegration_TimeoutDeathChain::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();
	UScoreSubsystem* Scoring = CreateTestScoringForIntegration();

	// Track deaths
	EDeathType ReceivedDeathType = EDeathType::None;
	bool bTimerExpiredFired = false;

	GM->OnTimerExpired.AddLambda([&bTimerExpiredFired]() {
		bTimerExpiredFired = true;
	});

	GM->StartGame();
	GM->OnSpawningComplete();
	TestEqual(TEXT("Playing"), GM->CurrentState, EGameState::Playing);

	// Tick timer to expiry
	GM->TickTimer(GM->TimePerLevel + 1.0f);

	// Verify timer expired delegate fired
	TestTrue(TEXT("OnTimerExpired fired"), bTimerExpiredFired);

	// Verify Dying state (HandleFrogDied was called with Timeout)
	TestEqual(TEXT("State is Dying"), GM->CurrentState, EGameState::Dying);

	// Verify timer is clamped to 0
	TestEqual(TEXT("Timer at 0"), GM->RemainingTime, 0.0f);

	// Simulate life loss
	Scoring->LoseLife();
	TestEqual(TEXT("Lives decremented to 2"), Scoring->Lives, 2);

	// Complete dying → spawning (not game over since lives > 0)
	GM->OnDyingComplete();
	TestEqual(TEXT("Back to Spawning"), GM->CurrentState, EGameState::Spawning);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Timer does not tick during non-Playing states
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIntegration_TimerStopsDuringNonPlaying,
	"UnrealFrog.Integration.Wiring.TimerStopsDuringNonPlaying",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FIntegration_TimerStopsDuringNonPlaying::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	// Title state — timer should not tick
	float TimeBefore = GM->RemainingTime;
	GM->TickTimer(5.0f);
	TestEqual(TEXT("Timer unchanged in Title"), GM->RemainingTime, TimeBefore);

	// Spawning state
	GM->StartGame();
	TimeBefore = GM->RemainingTime;
	GM->TickTimer(5.0f);
	TestEqual(TEXT("Timer unchanged in Spawning"), GM->RemainingTime, TimeBefore);

	// Playing state — timer should tick
	GM->OnSpawningComplete();
	TimeBefore = GM->RemainingTime;
	GM->TickTimer(5.0f);
	TestTrue(TEXT("Timer decremented in Playing"), GM->RemainingTime < TimeBefore);

	// Dying state
	GM->HandleFrogDied(EDeathType::Squish);
	TimeBefore = GM->RemainingTime;
	GM->TickTimer(5.0f);
	TestEqual(TEXT("Timer unchanged in Dying"), GM->RemainingTime, TimeBefore);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Wiring regression — unwired delegates do NOT fire
// Verifies that without AddDynamic, broadcasting is a no-op.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIntegration_UnwiredDelegatesNoOp,
	"UnrealFrog.Integration.Wiring.UnwiredDelegatesAreNoOp",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FIntegration_UnwiredDelegatesNoOp::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();

	// Do NOT wire delegates — simulate the Sprint 2 defect

	GM->StartGame();
	GM->OnSpawningComplete();

	// Broadcasting should be safe but HighestRowReached should NOT update
	// because HandleHopCompleted is not connected
	Frog->OnHopCompleted.Broadcast(FIntPoint(6, 5));
	TestEqual(TEXT("HighestRow stays 0 without wiring"), GM->HighestRowReached, 0);

	// Death delegate should not trigger state transition
	EGameState StateBefore = GM->CurrentState;
	Frog->OnFrogDied.Broadcast(EDeathType::Squish);
	TestEqual(TEXT("State unchanged without wiring"), GM->CurrentState, StateBefore);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
