// Copyright UnrealFrog Team. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Core/UnrealFrogGameMode.h"

#if WITH_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Test: Initial state matches design spec defaults
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameState_InitialState,
	"UnrealFrog.GameState.InitialState",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameState_InitialState::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	TestEqual(TEXT("Starts in Menu state"), GM->CurrentState, EGameState::Menu);
	TestEqual(TEXT("Wave starts at 1"), GM->CurrentWave, 1);
	TestEqual(TEXT("No homes filled"), GM->HomeSlotsFilledCount, 0);
	TestEqual(TEXT("TimePerLevel default 30"), GM->TimePerLevel, 30.0f);
	TestEqual(TEXT("RemainingTime default 30"), GM->RemainingTime, 30.0f);
	TestEqual(TEXT("TotalHomeSlots default 5"), GM->TotalHomeSlots, 5);
	TestEqual(TEXT("Home slot columns count"), GM->HomeSlotColumns.Num(), 5);
	TestEqual(TEXT("Home slot column 0"), GM->HomeSlotColumns[0], 1);
	TestEqual(TEXT("Home slot column 1"), GM->HomeSlotColumns[1], 4);
	TestEqual(TEXT("Home slot column 2"), GM->HomeSlotColumns[2], 6);
	TestEqual(TEXT("Home slot column 3"), GM->HomeSlotColumns[3], 8);
	TestEqual(TEXT("Home slot column 4"), GM->HomeSlotColumns[4], 11);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Menu -> Playing transition via StartGame
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameState_StartGame,
	"UnrealFrog.GameState.StartGame",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameState_StartGame::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	TestEqual(TEXT("Starts in Menu"), GM->CurrentState, EGameState::Menu);

	GM->StartGame();
	TestEqual(TEXT("Transitions to Playing"), GM->CurrentState, EGameState::Playing);
	TestEqual(TEXT("Timer reset to TimePerLevel"), GM->RemainingTime, GM->TimePerLevel);
	TestEqual(TEXT("Wave is 1"), GM->CurrentWave, 1);
	TestEqual(TEXT("Homes filled reset to 0"), GM->HomeSlotsFilledCount, 0);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Playing -> Paused -> Playing transitions
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameState_PauseResume,
	"UnrealFrog.GameState.PauseResume",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameState_PauseResume::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	TestEqual(TEXT("Playing state"), GM->CurrentState, EGameState::Playing);

	// Simulate some time passing
	GM->TickTimer(5.0f);
	float TimeBeforePause = GM->RemainingTime;

	GM->PauseGame();
	TestEqual(TEXT("Paused state"), GM->CurrentState, EGameState::Paused);

	// Timer should not tick while paused
	GM->TickTimer(10.0f);
	TestEqual(TEXT("Timer unchanged while paused"), GM->RemainingTime, TimeBeforePause);

	GM->ResumeGame();
	TestEqual(TEXT("Back to Playing"), GM->CurrentState, EGameState::Playing);

	// Pause from Menu should not work
	AUnrealFrogGameMode* GM2 = NewObject<AUnrealFrogGameMode>();
	GM2->PauseGame();
	TestEqual(TEXT("Cannot pause from Menu"), GM2->CurrentState, EGameState::Menu);

	return true;
}

// ---------------------------------------------------------------------------
// Test: GameOver when lives reach 0
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameState_GameOver,
	"UnrealFrog.GameState.GameOver",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameState_GameOver::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	TestEqual(TEXT("Playing state"), GM->CurrentState, EGameState::Playing);

	// Trigger game over
	GM->HandleGameOver();
	TestEqual(TEXT("GameOver state"), GM->CurrentState, EGameState::GameOver);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Cannot start game from GameOver; must ReturnToMenu first
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameState_CannotStartFromGameOver,
	"UnrealFrog.GameState.CannotStartFromGameOver",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameState_CannotStartFromGameOver::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	GM->HandleGameOver();
	TestEqual(TEXT("In GameOver"), GM->CurrentState, EGameState::GameOver);

	// Attempt to start directly from GameOver -- should fail
	GM->StartGame();
	TestEqual(TEXT("Still GameOver"), GM->CurrentState, EGameState::GameOver);

	// ReturnToMenu first, then StartGame should work
	GM->ReturnToMenu();
	TestEqual(TEXT("Back to Menu"), GM->CurrentState, EGameState::Menu);

	GM->StartGame();
	TestEqual(TEXT("Now Playing"), GM->CurrentState, EGameState::Playing);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Level timer counts down and timeout triggers death
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameState_LevelTimer,
	"UnrealFrog.GameState.LevelTimer",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameState_LevelTimer::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	TestEqual(TEXT("Timer starts at 30"), GM->RemainingTime, 30.0f);

	GM->TickTimer(10.0f);
	TestNearlyEqual(TEXT("Timer after 10s"), GM->RemainingTime, 20.0f);

	GM->TickTimer(15.0f);
	TestNearlyEqual(TEXT("Timer after 25s"), GM->RemainingTime, 5.0f);

	// Track if timeout was triggered
	bool bTimedOut = false;
	GM->OnTimerExpired.AddLambda([&bTimedOut]() { bTimedOut = true; });

	GM->TickTimer(6.0f); // push past 0
	TestTrue(TEXT("Timer expired event fired"), bTimedOut);
	TestTrue(TEXT("Timer clamped at or below 0"), GM->RemainingTime <= 0.0f);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Reaching a home slot fills it and increments count
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameState_HomeSlotFill,
	"UnrealFrog.GameState.HomeSlotFill",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameState_HomeSlotFill::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();

	// Column 1 is a valid home slot
	TestTrue(TEXT("Column 1 is home slot"), GM->IsHomeSlotColumn(1));
	TestTrue(TEXT("Column 4 is home slot"), GM->IsHomeSlotColumn(4));
	TestFalse(TEXT("Column 3 is NOT home slot"), GM->IsHomeSlotColumn(3));

	// Fill the first slot
	bool bFilled = GM->TryFillHomeSlot(1);
	TestTrue(TEXT("Successfully filled slot at column 1"), bFilled);
	TestEqual(TEXT("HomeSlotsFilledCount is 1"), GM->HomeSlotsFilledCount, 1);

	// Trying to fill the same slot again should fail
	bFilled = GM->TryFillHomeSlot(1);
	TestFalse(TEXT("Cannot fill same slot twice"), bFilled);
	TestEqual(TEXT("HomeSlotsFilledCount still 1"), GM->HomeSlotsFilledCount, 1);

	// Invalid column should fail
	bFilled = GM->TryFillHomeSlot(3);
	TestFalse(TEXT("Invalid column fails"), bFilled);

	// Fill another valid slot
	bFilled = GM->TryFillHomeSlot(6);
	TestTrue(TEXT("Filled slot at column 6"), bFilled);
	TestEqual(TEXT("HomeSlotsFilledCount is 2"), GM->HomeSlotsFilledCount, 2);

	return true;
}

// ---------------------------------------------------------------------------
// Test: All 5 homes filled completes wave and increments wave number
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameState_WaveComplete,
	"UnrealFrog.GameState.WaveComplete",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameState_WaveComplete::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	TestEqual(TEXT("Wave starts at 1"), GM->CurrentWave, 1);

	// Fill 4 home slots -- wave should NOT be complete yet
	GM->TryFillHomeSlot(1);
	GM->TryFillHomeSlot(4);
	GM->TryFillHomeSlot(6);
	GM->TryFillHomeSlot(8);
	TestEqual(TEXT("Still wave 1 at 4 slots"), GM->CurrentWave, 1);
	TestEqual(TEXT("4 slots filled"), GM->HomeSlotsFilledCount, 4);

	// Fill the 5th slot -- triggers wave complete
	GM->TryFillHomeSlot(11);
	TestEqual(TEXT("CurrentWave is now 2"), GM->CurrentWave, 2);
	TestEqual(TEXT("Homes reset to 0"), GM->HomeSlotsFilledCount, 0);

	// Verify we can fill slots again in wave 2
	bool bFilled = GM->TryFillHomeSlot(1);
	TestTrue(TEXT("Can fill slot 1 again in wave 2"), bFilled);
	TestEqual(TEXT("1 slot filled in wave 2"), GM->HomeSlotsFilledCount, 1);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Difficulty scaling -- speed multiplier and gap reduction per wave
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameState_DifficultyScaling,
	"UnrealFrog.GameState.DifficultyScaling",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameState_DifficultyScaling::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	// Wave 1: speed 1.0, gap reduction 0
	GM->CurrentWave = 1;
	TestNearlyEqual(TEXT("Wave 1 speed multiplier"), GM->GetSpeedMultiplier(), 1.0f);
	TestEqual(TEXT("Wave 1 gap reduction"), GM->GetGapReduction(), 0);

	// Wave 2: speed 1.1, gap reduction 0
	GM->CurrentWave = 2;
	TestNearlyEqual(TEXT("Wave 2 speed multiplier"), GM->GetSpeedMultiplier(), 1.1f);
	TestEqual(TEXT("Wave 2 gap reduction"), GM->GetGapReduction(), 0);

	// Wave 4: speed 1.3, gap reduction 1
	GM->CurrentWave = 4;
	TestNearlyEqual(TEXT("Wave 4 speed multiplier"), GM->GetSpeedMultiplier(), 1.3f);
	TestEqual(TEXT("Wave 4 gap reduction"), GM->GetGapReduction(), 1);

	// Wave 7: speed 1.6, gap reduction 2
	GM->CurrentWave = 7;
	TestNearlyEqual(TEXT("Wave 7 speed multiplier"), GM->GetSpeedMultiplier(), 1.6f);
	TestEqual(TEXT("Wave 7 gap reduction"), GM->GetGapReduction(), 2);

	// Wave 10: speed 1.9, gap reduction 3
	GM->CurrentWave = 10;
	TestNearlyEqual(TEXT("Wave 10 speed multiplier"), GM->GetSpeedMultiplier(), 1.9f);
	TestEqual(TEXT("Wave 10 gap reduction"), GM->GetGapReduction(), 3);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
