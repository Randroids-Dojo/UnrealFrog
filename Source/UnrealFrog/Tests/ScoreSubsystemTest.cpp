// Copyright UnrealFrog Team. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#include "Core/ScoreSubsystem.h"

#if WITH_AUTOMATION_TESTS

namespace
{
	UScoreSubsystem* CreateTestScoreSubsystem()
	{
		UGameInstance* TestGI = NewObject<UGameInstance>();
		return NewObject<UScoreSubsystem>(TestGI);
	}
}

// ---------------------------------------------------------------------------
// Test: Initial state matches design spec defaults
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScoreSubsystem_InitialState,
	"UnrealFrog.Score.InitialState",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FScoreSubsystem_InitialState::RunTest(const FString& Parameters)
{
	UScoreSubsystem* Scoring = CreateTestScoreSubsystem();

	TestEqual(TEXT("Score starts at 0"), Scoring->Score, 0);
	TestEqual(TEXT("HighScore starts at 0"), Scoring->HighScore, 0);
	TestEqual(TEXT("Lives starts at 3"), Scoring->Lives, 3);
	TestNearlyEqual(TEXT("Multiplier starts at 1.0"), Scoring->Multiplier, 1.0f);
	TestEqual(TEXT("PointsPerHop default"), Scoring->PointsPerHop, 10);
	TestNearlyEqual(TEXT("MultiplierIncrement default"), Scoring->MultiplierIncrement, 1.0f);
	TestEqual(TEXT("ExtraLifeThreshold default"), Scoring->ExtraLifeThreshold, 10000);
	TestEqual(TEXT("MaxLives default"), Scoring->MaxLives, 9);
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
	UScoreSubsystem* Scoring = CreateTestScoreSubsystem();

	// First hop: 10 * 1.0 = 10
	Scoring->AddForwardHopScore();
	TestEqual(TEXT("Score after first hop"), Scoring->Score, 10);

	// Second hop: multiplier increased to 2.0, so 10 * 2.0 = 20 (total 30)
	Scoring->AddForwardHopScore();
	TestEqual(TEXT("Score after second hop"), Scoring->Score, 30);

	// Third hop: multiplier 3.0, so 10 * 3.0 = 30 (total 60)
	Scoring->AddForwardHopScore();
	TestEqual(TEXT("Score after third hop"), Scoring->Score, 60);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Consecutive forward hops increase multiplier by 1.0 each time
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScoreSubsystem_MultiplierIncrease,
	"UnrealFrog.Score.MultiplierIncrease",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FScoreSubsystem_MultiplierIncrease::RunTest(const FString& Parameters)
{
	UScoreSubsystem* Scoring = CreateTestScoreSubsystem();

	TestNearlyEqual(TEXT("Multiplier starts at 1.0"), Scoring->Multiplier, 1.0f);

	Scoring->AddForwardHopScore();
	TestNearlyEqual(TEXT("Multiplier after 1st hop"), Scoring->Multiplier, 2.0f);

	Scoring->AddForwardHopScore();
	TestNearlyEqual(TEXT("Multiplier after 2nd hop"), Scoring->Multiplier, 3.0f);

	Scoring->AddForwardHopScore();
	TestNearlyEqual(TEXT("Multiplier after 3rd hop"), Scoring->Multiplier, 4.0f);

	Scoring->AddForwardHopScore();
	TestNearlyEqual(TEXT("Multiplier after 4th hop"), Scoring->Multiplier, 5.0f);

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
	UScoreSubsystem* Scoring = CreateTestScoreSubsystem();

	// Build up multiplier
	Scoring->AddForwardHopScore();
	Scoring->AddForwardHopScore();
	TestNearlyEqual(TEXT("Multiplier is 3.0 before reset"), Scoring->Multiplier, 3.0f);

	// Retreat resets multiplier
	Scoring->ResetMultiplier();
	TestNearlyEqual(TEXT("Multiplier after retreat reset"), Scoring->Multiplier, 1.0f);

	// Build up again
	Scoring->AddForwardHopScore();
	Scoring->AddForwardHopScore();
	TestNearlyEqual(TEXT("Multiplier rebuilt to 3.0"), Scoring->Multiplier, 3.0f);

	// Death also resets multiplier (via LoseLife)
	Scoring->LoseLife();
	TestNearlyEqual(TEXT("Multiplier after death"), Scoring->Multiplier, 1.0f);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Time bonus calculation: floor(RemainingSeconds) * 10
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScoreSubsystem_TimeBonus,
	"UnrealFrog.Score.TimeBonus",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FScoreSubsystem_TimeBonus::RunTest(const FString& Parameters)
{
	UScoreSubsystem* Scoring = CreateTestScoreSubsystem();

	// 20 seconds remaining: floor(20) * 10 = 200
	Scoring->AddTimeBonus(20.0f);
	TestEqual(TEXT("20s time bonus"), Scoring->Score, 200);

	// 15.5 seconds remaining: floor(15.5) * 10 = 150
	Scoring->Score = 0;
	Scoring->AddTimeBonus(15.5f);
	TestEqual(TEXT("15.5s time bonus (floored)"), Scoring->Score, 150);

	// 0 seconds remaining: guard returns early, no points
	Scoring->Score = 0;
	Scoring->AddTimeBonus(0.0f);
	TestEqual(TEXT("Zero time bonus"), Scoring->Score, 0);

	// Negative seconds: guard returns early, no points
	Scoring->Score = 0;
	Scoring->AddTimeBonus(-5.0f);
	TestEqual(TEXT("Negative time bonus"), Scoring->Score, 0);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Extra life at 10,000 threshold, capped at MaxLives (9)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScoreSubsystem_ExtraLife,
	"UnrealFrog.Score.ExtraLife",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FScoreSubsystem_ExtraLife::RunTest(const FString& Parameters)
{
	UScoreSubsystem* Scoring = CreateTestScoreSubsystem();

	TestEqual(TEXT("Initial lives"), Scoring->Lives, 3);

	// Push score just below threshold -- no extra life
	// With MultiplierIncrement=1.0: hop1 = 10*1.0=10, multiplier->2.0
	Scoring->Score = 9989;
	Scoring->AddForwardHopScore(); // +10, score = 9999, multiplier = 2.0
	TestEqual(TEXT("No extra life below threshold"), Scoring->Lives, 3);

	// Cross threshold -- extra life awarded
	// hop2 = 10*2.0=20, score = 10019, multiplier->3.0
	Scoring->AddForwardHopScore();
	TestEqual(TEXT("Extra life at 10000"), Scoring->Lives, 4);

	// Push to 20,000 threshold -- another extra life
	Scoring->Score = 19989;
	Scoring->Multiplier = 1.0f;
	Scoring->AddForwardHopScore(); // +10, score = 19999, multiplier->2.0
	TestEqual(TEXT("No extra life below 20000"), Scoring->Lives, 4);

	Scoring->AddForwardHopScore(); // +20 (multiplier 2.0), score = 20019, multiplier->3.0
	TestEqual(TEXT("Extra life at 20000"), Scoring->Lives, 5);

	// MaxLives is 9; set lives to 9 and verify no more extra lives
	Scoring->Lives = 9;
	Scoring->Score = 29989;
	Scoring->Multiplier = 1.0f;
	Scoring->AddForwardHopScore();
	Scoring->AddForwardHopScore();
	TestEqual(TEXT("Capped at MaxLives"), Scoring->Lives, 9);

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
	UScoreSubsystem* Scoring = CreateTestScoreSubsystem();

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
	UScoreSubsystem* Scoring = CreateTestScoreSubsystem();

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
	UScoreSubsystem* Scoring = CreateTestScoreSubsystem();

	TestEqual(TEXT("High score starts at 0"), Scoring->HighScore, 0);

	Scoring->AddForwardHopScore(); // +10
	TestEqual(TEXT("High score updates to 10"), Scoring->HighScore, 10);

	Scoring->AddForwardHopScore(); // +20 (multiplier 2.0)
	TestEqual(TEXT("High score updates to 30"), Scoring->HighScore, 30);

	// Start a new game -- high score should persist
	Scoring->StartNewGame();
	TestEqual(TEXT("High score persists after new game"), Scoring->HighScore, 30);

	// Score below high score -- high score stays
	Scoring->AddForwardHopScore(); // +10, score = 10
	TestEqual(TEXT("High score unchanged when score is lower"), Scoring->HighScore, 30);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Multiplier caps at MaxMultiplier (5.0)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScoreSubsystem_MultiplierCap,
	"UnrealFrog.Score.MultiplierCap",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FScoreSubsystem_MultiplierCap::RunTest(const FString& Parameters)
{
	UScoreSubsystem* Scoring = CreateTestScoreSubsystem();

	// Hop 1: mult 1.0 -> awards 10, mult becomes 2.0
	Scoring->AddForwardHopScore();
	// Hop 2: mult 2.0 -> awards 20, mult becomes 3.0
	Scoring->AddForwardHopScore();
	// Hop 3: mult 3.0 -> awards 30, mult becomes 4.0
	Scoring->AddForwardHopScore();
	// Hop 4: mult 4.0 -> awards 40, mult becomes 5.0 (capped)
	Scoring->AddForwardHopScore();
	TestNearlyEqual(TEXT("Multiplier at cap after 4 hops"), Scoring->Multiplier, 5.0f);

	// Hop 5: mult 5.0 -> awards 50, mult stays at 5.0 (capped at MaxMultiplier)
	Scoring->AddForwardHopScore();
	TestNearlyEqual(TEXT("Multiplier still capped after 5 hops"), Scoring->Multiplier, 5.0f);

	// Hop 6: mult 5.0 -> awards 50, still capped
	int32 ScoreBefore = Scoring->Score;
	Scoring->AddForwardHopScore();
	TestEqual(TEXT("Hop 6 awards 50 at cap"), Scoring->Score - ScoreBefore, 50);
	TestNearlyEqual(TEXT("Multiplier still 5.0 after 6 hops"), Scoring->Multiplier, 5.0f);

	return true;
}

// ---------------------------------------------------------------------------
// Test: AddBonusPoints adds arbitrary points
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScoreSubsystem_BonusPoints,
	"UnrealFrog.Score.BonusPoints",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FScoreSubsystem_BonusPoints::RunTest(const FString& Parameters)
{
	UScoreSubsystem* Scoring = CreateTestScoreSubsystem();

	Scoring->AddBonusPoints(200);
	TestEqual(TEXT("Score after bonus"), Scoring->Score, 200);

	Scoring->AddBonusPoints(1000);
	TestEqual(TEXT("Score after round bonus"), Scoring->Score, 1200);

	return true;
}

// ---------------------------------------------------------------------------
// Test: AddHomeSlotScore awards HomeSlotPoints (200)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScoreSubsystem_HomeSlotScore,
	"UnrealFrog.Score.HomeSlotScore",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FScoreSubsystem_HomeSlotScore::RunTest(const FString& Parameters)
{
	UScoreSubsystem* Scoring = CreateTestScoreSubsystem();

	Scoring->AddHomeSlotScore();
	TestEqual(TEXT("Home slot awards 200"), Scoring->Score, 200);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
