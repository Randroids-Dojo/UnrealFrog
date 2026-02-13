// Copyright UnrealFrog Team. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Core/ScoreSubsystem.h"

#if WITH_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Test: Initial state matches design spec defaults
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScoreSubsystem_InitialState,
	"UnrealFrog.Score.InitialState",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FScoreSubsystem_InitialState::RunTest(const FString& Parameters)
{
	UScoreSubsystem* Scoring = NewObject<UScoreSubsystem>();

	TestEqual(TEXT("Score starts at 0"), Scoring->Score, 0);
	TestEqual(TEXT("HighScore starts at 0"), Scoring->HighScore, 0);
	TestEqual(TEXT("Lives starts at 3"), Scoring->Lives, 3);
	TestNearlyEqual(TEXT("Multiplier starts at 1.0"), Scoring->Multiplier, 1.0f);
	TestEqual(TEXT("PointsPerHop default"), Scoring->PointsPerHop, 10);
	TestNearlyEqual(TEXT("MultiplierIncrement default"), Scoring->MultiplierIncrement, 0.5f);
	TestEqual(TEXT("ExtraLifeThreshold default"), Scoring->ExtraLifeThreshold, 10000);
	TestEqual(TEXT("MaxLives default"), Scoring->MaxLives, 5);
	TestEqual(TEXT("InitialLives default"), Scoring->InitialLives, 3);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Forward hop awards PointsPerHop * Multiplier
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScoreSubsystem_ForwardHopScoring,
	"UnrealFrog.Score.ForwardHopScoring",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FScoreSubsystem_ForwardHopScoring::RunTest(const FString& Parameters)
{
	UScoreSubsystem* Scoring = NewObject<UScoreSubsystem>();

	// First hop: 10 * 1.0 = 10
	Scoring->AddForwardHopScore();
	TestEqual(TEXT("Score after first hop"), Scoring->Score, 10);

	// Second hop: multiplier should have increased to 1.5, so 10 * 1.5 = 15
	Scoring->AddForwardHopScore();
	TestEqual(TEXT("Score after second hop"), Scoring->Score, 25);

	// Third hop: multiplier 2.0, so 10 * 2.0 = 20
	Scoring->AddForwardHopScore();
	TestEqual(TEXT("Score after third hop"), Scoring->Score, 45);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Consecutive forward hops increase multiplier by 0.5 each time
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScoreSubsystem_MultiplierIncrease,
	"UnrealFrog.Score.MultiplierIncrease",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FScoreSubsystem_MultiplierIncrease::RunTest(const FString& Parameters)
{
	UScoreSubsystem* Scoring = NewObject<UScoreSubsystem>();

	TestNearlyEqual(TEXT("Multiplier starts at 1.0"), Scoring->Multiplier, 1.0f);

	Scoring->AddForwardHopScore();
	TestNearlyEqual(TEXT("Multiplier after 1st hop"), Scoring->Multiplier, 1.5f);

	Scoring->AddForwardHopScore();
	TestNearlyEqual(TEXT("Multiplier after 2nd hop"), Scoring->Multiplier, 2.0f);

	Scoring->AddForwardHopScore();
	TestNearlyEqual(TEXT("Multiplier after 3rd hop"), Scoring->Multiplier, 2.5f);

	Scoring->AddForwardHopScore();
	TestNearlyEqual(TEXT("Multiplier after 4th hop"), Scoring->Multiplier, 3.0f);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Retreat and death reset the multiplier to 1.0
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScoreSubsystem_MultiplierReset,
	"UnrealFrog.Score.MultiplierReset",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FScoreSubsystem_MultiplierReset::RunTest(const FString& Parameters)
{
	UScoreSubsystem* Scoring = NewObject<UScoreSubsystem>();

	// Build up multiplier
	Scoring->AddForwardHopScore();
	Scoring->AddForwardHopScore();
	TestNearlyEqual(TEXT("Multiplier is 2.0 before reset"), Scoring->Multiplier, 2.0f);

	// Retreat resets multiplier
	Scoring->ResetMultiplier();
	TestNearlyEqual(TEXT("Multiplier after retreat reset"), Scoring->Multiplier, 1.0f);

	// Build up again
	Scoring->AddForwardHopScore();
	Scoring->AddForwardHopScore();
	TestNearlyEqual(TEXT("Multiplier rebuilt to 2.0"), Scoring->Multiplier, 2.0f);

	// Death also resets multiplier (via LoseLife)
	Scoring->LoseLife();
	TestNearlyEqual(TEXT("Multiplier after death"), Scoring->Multiplier, 1.0f);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Time bonus calculation: (RemainingTime / MaxTime) * 1000
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScoreSubsystem_TimeBonus,
	"UnrealFrog.Score.TimeBonus",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FScoreSubsystem_TimeBonus::RunTest(const FString& Parameters)
{
	UScoreSubsystem* Scoring = NewObject<UScoreSubsystem>();

	// Full time remaining: (60 / 60) * 1000 = 1000
	Scoring->AddTimeBonus(60.0f, 60.0f);
	TestEqual(TEXT("Full time bonus"), Scoring->Score, 1000);

	// Half time remaining: (30 / 60) * 1000 = 500
	Scoring->Score = 0; // Reset for clarity
	Scoring->AddTimeBonus(30.0f, 60.0f);
	TestEqual(TEXT("Half time bonus"), Scoring->Score, 500);

	// No time remaining: 0
	Scoring->Score = 0;
	Scoring->AddTimeBonus(0.0f, 60.0f);
	TestEqual(TEXT("Zero time bonus"), Scoring->Score, 0);

	// Edge case: MaxTime is 0 should not crash, awards 0
	Scoring->Score = 0;
	Scoring->AddTimeBonus(10.0f, 0.0f);
	TestEqual(TEXT("MaxTime zero edge case"), Scoring->Score, 0);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Extra life at 10,000 threshold, capped at MaxLives (5)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScoreSubsystem_ExtraLife,
	"UnrealFrog.Score.ExtraLife",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FScoreSubsystem_ExtraLife::RunTest(const FString& Parameters)
{
	UScoreSubsystem* Scoring = NewObject<UScoreSubsystem>();

	TestEqual(TEXT("Initial lives"), Scoring->Lives, 3);

	// Push score just below threshold -- no extra life
	Scoring->Score = 9989;
	Scoring->AddForwardHopScore(); // +10, score = 9999
	TestEqual(TEXT("No extra life below threshold"), Scoring->Lives, 3);

	// Cross threshold -- extra life awarded
	Scoring->AddForwardHopScore(); // +15 (multiplier is 1.5), score = 10014
	TestEqual(TEXT("Extra life at 10000"), Scoring->Lives, 4);

	// Push to 20,000 threshold -- another extra life
	Scoring->Score = 19989;
	Scoring->Multiplier = 1.0f;
	Scoring->AddForwardHopScore(); // +10, score = 19999
	TestEqual(TEXT("No extra life below 20000"), Scoring->Lives, 4);

	Scoring->AddForwardHopScore(); // +15 (multiplier 1.5), score = 20014
	TestEqual(TEXT("Extra life at 20000"), Scoring->Lives, 5);

	// At max lives -- no more extra lives
	Scoring->Score = 29989;
	Scoring->Multiplier = 1.0f;
	Scoring->AddForwardHopScore();
	Scoring->AddForwardHopScore();
	TestEqual(TEXT("Capped at MaxLives"), Scoring->Lives, 5);

	return true;
}

// ---------------------------------------------------------------------------
// Test: LoseLife decrements lives and triggers game over at 0
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScoreSubsystem_LoseLife,
	"UnrealFrog.Score.LoseLife",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FScoreSubsystem_LoseLife::RunTest(const FString& Parameters)
{
	UScoreSubsystem* Scoring = NewObject<UScoreSubsystem>();

	TestEqual(TEXT("Start with 3 lives"), Scoring->Lives, 3);
	TestFalse(TEXT("Not game over"), Scoring->IsGameOver());

	Scoring->LoseLife();
	TestEqual(TEXT("2 lives after first death"), Scoring->Lives, 2);
	TestFalse(TEXT("Not game over yet"), Scoring->IsGameOver());

	Scoring->LoseLife();
	TestEqual(TEXT("1 life after second death"), Scoring->Lives, 1);
	TestFalse(TEXT("Still not game over"), Scoring->IsGameOver());

	Scoring->LoseLife();
	TestEqual(TEXT("0 lives after third death"), Scoring->Lives, 0);
	TestTrue(TEXT("Game over at 0 lives"), Scoring->IsGameOver());

	return true;
}

// ---------------------------------------------------------------------------
// Test: StartNewGame resets score and lives, but high score persists
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScoreSubsystem_NewGame,
	"UnrealFrog.Score.NewGame",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FScoreSubsystem_NewGame::RunTest(const FString& Parameters)
{
	UScoreSubsystem* Scoring = NewObject<UScoreSubsystem>();

	// Accumulate some score
	Scoring->Score = 5000;
	Scoring->HighScore = 5000;
	Scoring->Lives = 1;
	Scoring->Multiplier = 3.0f;

	Scoring->StartNewGame();

	TestEqual(TEXT("Score resets to 0"), Scoring->Score, 0);
	TestEqual(TEXT("High score persists"), Scoring->HighScore, 5000);
	TestEqual(TEXT("Lives reset to InitialLives"), Scoring->Lives, 3);
	TestNearlyEqual(TEXT("Multiplier resets to 1.0"), Scoring->Multiplier, 1.0f);
	TestFalse(TEXT("Not game over after new game"), Scoring->IsGameOver());

	return true;
}

// ---------------------------------------------------------------------------
// Test: High score updates when current score exceeds it
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScoreSubsystem_HighScore,
	"UnrealFrog.Score.HighScore",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FScoreSubsystem_HighScore::RunTest(const FString& Parameters)
{
	UScoreSubsystem* Scoring = NewObject<UScoreSubsystem>();

	TestEqual(TEXT("High score starts at 0"), Scoring->HighScore, 0);

	Scoring->AddForwardHopScore(); // +10
	TestEqual(TEXT("High score updates to 10"), Scoring->HighScore, 10);

	Scoring->AddForwardHopScore(); // +15
	TestEqual(TEXT("High score updates to 25"), Scoring->HighScore, 25);

	// Start a new game -- high score should persist
	Scoring->StartNewGame();
	TestEqual(TEXT("High score persists after new game"), Scoring->HighScore, 25);

	// Score below high score -- high score stays
	Scoring->AddForwardHopScore(); // +10, score = 10
	TestEqual(TEXT("High score unchanged when score is lower"), Scoring->HighScore, 25);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
