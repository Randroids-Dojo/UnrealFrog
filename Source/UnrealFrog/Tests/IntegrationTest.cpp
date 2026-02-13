// Copyright UnrealFrog Team. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Core/FrogCharacter.h"
#include "Core/HazardBase.h"
#include "Core/UnrealFrogGameMode.h"

#if WITH_AUTOMATION_TESTS

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

#endif // WITH_AUTOMATION_TESTS
