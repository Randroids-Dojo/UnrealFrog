// Copyright UnrealFrog Team. All Rights Reserved.
//
// PlayUnreal E2E Test Harness
//
// End-to-end scenario tests that exercise the full game loop via direct method
// calls on GameMode, FrogCharacter, and ScoreSubsystem. Each scenario simulates
// a complete gameplay sequence and verifies state transitions + scoring.
//
// Structured [PlayUnreal] log output enables run-tests.sh --e2e parsing.
//
// Sprint 4, Task #2 — DevOps Engineer

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#include "Core/FrogCharacter.h"
#include "Core/HazardBase.h"
#include "Core/ScoreSubsystem.h"
#include "Core/UnrealFrogGameMode.h"

#if WITH_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Helper: structured state logging for PlayUnreal parsing
// ---------------------------------------------------------------------------
namespace PlayUnreal
{
	void LogState(FAutomationTestBase* Test, AUnrealFrogGameMode* GM,
		UScoreSubsystem* Scoring, AFrogCharacter* Frog)
	{
		auto StateToString = [](EGameState S) -> const TCHAR* {
			switch (S)
			{
			case EGameState::Title:			return TEXT("Title");
			case EGameState::Spawning:		return TEXT("Spawning");
			case EGameState::Playing:		return TEXT("Playing");
			case EGameState::Paused:		return TEXT("Paused");
			case EGameState::Dying:			return TEXT("Dying");
			case EGameState::RoundComplete:	return TEXT("RoundComplete");
			case EGameState::GameOver:		return TEXT("GameOver");
			default:						return TEXT("Unknown");
			}
		};

		int32 Score = Scoring ? Scoring->Score : -1;
		int32 Lives = Scoring ? Scoring->Lives : -1;
		int32 Wave = GM ? GM->CurrentWave : -1;
		const TCHAR* State = GM ? StateToString(GM->CurrentState) : TEXT("NoGM");
		FIntPoint FrogPos = Frog ? Frog->GridPosition : FIntPoint(-1, -1);

		UE_LOG(LogTemp, Display,
			TEXT("[PlayUnreal] SCORE=%d LIVES=%d WAVE=%d STATE=%s FROG_POS=(%d,%d)"),
			Score, Lives, Wave, State, FrogPos.X, FrogPos.Y);
	}

	UScoreSubsystem* CreateScoring()
	{
		UGameInstance* GI = NewObject<UGameInstance>();
		return NewObject<UScoreSubsystem>(GI);
	}
}

// ===========================================================================
// Scenario 1: Forward Progress
// Hop up 5 times. Score increases, highest row tracks correctly.
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayUnreal_ForwardProgress,
	"UnrealFrog.PlayUnreal.Scenario1_ForwardProgress",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPlayUnreal_ForwardProgress::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	UScoreSubsystem* Scoring = PlayUnreal::CreateScoring();

	UE_LOG(LogTemp, Display, TEXT("[PlayUnreal] === Scenario 1: Forward Progress ==="));

	// Start the game
	GM->StartGame();
	GM->OnSpawningComplete();
	TestEqual(TEXT("State is Playing"), GM->CurrentState, EGameState::Playing);
	PlayUnreal::LogState(this, GM, Scoring, Frog);

	// Hop forward 5 times, scoring each new row
	for (int32 Row = 1; Row <= 5; Row++)
	{
		GM->HandleHopCompleted(FIntPoint(6, Row));
		Scoring->AddForwardHopScore();

		TestEqual(
			*FString::Printf(TEXT("Highest row is %d"), Row),
			GM->HighestRowReached, Row);
	}

	// Verify score increased: 5 hops with increasing multiplier
	// Hop 1: 10*1=10, Hop 2: 10*2=20, Hop 3: 10*3=30, Hop 4: 10*4=40, Hop 5: 10*5=50
	// Total = 150
	TestEqual(TEXT("Score after 5 forward hops"), Scoring->Score, 150);
	TestEqual(TEXT("Multiplier is 5.0 (capped)"), Scoring->Multiplier, 5.0f);
	TestEqual(TEXT("Highest row reached is 5"), GM->HighestRowReached, 5);
	TestEqual(TEXT("Still playing"), GM->CurrentState, EGameState::Playing);

	PlayUnreal::LogState(this, GM, Scoring, Frog);
	UE_LOG(LogTemp, Display, TEXT("[PlayUnreal] === Scenario 1: PASSED ==="));

	return true;
}

// ===========================================================================
// Scenario 2: Road Death
// Hop into a road lane, encounter a hazard. Verify death cycle.
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayUnreal_RoadDeath,
	"UnrealFrog.PlayUnreal.Scenario2_RoadDeath",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPlayUnreal_RoadDeath::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	UScoreSubsystem* Scoring = PlayUnreal::CreateScoring();

	UE_LOG(LogTemp, Display, TEXT("[PlayUnreal] === Scenario 2: Road Death ==="));

	GM->StartGame();
	GM->OnSpawningComplete();
	TestEqual(TEXT("Playing"), GM->CurrentState, EGameState::Playing);
	TestEqual(TEXT("Lives start at 3"), Scoring->Lives, 3);
	PlayUnreal::LogState(this, GM, Scoring, Frog);

	// Hop forward into road area
	GM->HandleHopCompleted(FIntPoint(6, 1));
	TestEqual(TEXT("Row 1 reached"), GM->HighestRowReached, 1);

	// Simulate car collision via HandleFrogDied (the real path through GameMode)
	GM->HandleFrogDied(EDeathType::Squish);
	TestEqual(TEXT("State is Dying"), GM->CurrentState, EGameState::Dying);

	// Lose a life via scoring subsystem (wired in real game)
	Scoring->LoseLife();
	TestEqual(TEXT("Lives decremented to 2"), Scoring->Lives, 2);
	PlayUnreal::LogState(this, GM, Scoring, Frog);

	// Complete dying animation
	GM->OnDyingComplete();
	TestEqual(TEXT("Back to Spawning"), GM->CurrentState, EGameState::Spawning);

	// Complete spawning
	GM->OnSpawningComplete();
	TestEqual(TEXT("Playing again"), GM->CurrentState, EGameState::Playing);

	// Verify highest row reset after death
	TestEqual(TEXT("Highest row reset to 0"), GM->HighestRowReached, 0);

	PlayUnreal::LogState(this, GM, Scoring, Frog);
	UE_LOG(LogTemp, Display, TEXT("[PlayUnreal] === Scenario 2: PASSED ==="));

	return true;
}

// ===========================================================================
// Scenario 3: Pause and Resume
// Hop, pause, verify timer freeze, unpause, verify timer resumes.
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayUnreal_PauseResume,
	"UnrealFrog.PlayUnreal.Scenario3_PauseResume",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPlayUnreal_PauseResume::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	UScoreSubsystem* Scoring = PlayUnreal::CreateScoring();

	UE_LOG(LogTemp, Display, TEXT("[PlayUnreal] === Scenario 3: Pause and Resume ==="));

	GM->StartGame();
	GM->OnSpawningComplete();
	TestEqual(TEXT("Playing"), GM->CurrentState, EGameState::Playing);

	// Hop forward
	GM->HandleHopCompleted(FIntPoint(6, 1));
	TestEqual(TEXT("Row 1 reached"), GM->HighestRowReached, 1);

	// Tick timer to consume some time
	GM->TickTimer(5.0f);
	float TimerBeforePause = GM->RemainingTime;
	TestTrue(TEXT("Timer decremented from playing"), TimerBeforePause < GM->TimePerLevel);
	PlayUnreal::LogState(this, GM, Scoring, Frog);

	// Pause the game
	GM->PauseGame();
	TestEqual(TEXT("Paused"), GM->CurrentState, EGameState::Paused);

	// Tick timer during pause — should NOT change
	GM->TickTimer(10.0f);
	TestEqual(TEXT("Timer unchanged during pause"), GM->RemainingTime, TimerBeforePause);
	PlayUnreal::LogState(this, GM, Scoring, Frog);

	// Resume the game
	GM->ResumeGame();
	TestEqual(TEXT("Playing after resume"), GM->CurrentState, EGameState::Playing);

	// Timer should resume ticking
	GM->TickTimer(2.0f);
	TestTrue(TEXT("Timer resumed after unpause"),
		GM->RemainingTime < TimerBeforePause);

	PlayUnreal::LogState(this, GM, Scoring, Frog);
	UE_LOG(LogTemp, Display, TEXT("[PlayUnreal] === Scenario 3: PASSED ==="));

	return true;
}

// ===========================================================================
// Scenario 4: Game Over
// Set lives to 1, trigger death, verify GameOver state.
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayUnreal_GameOver,
	"UnrealFrog.PlayUnreal.Scenario4_GameOver",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPlayUnreal_GameOver::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	UScoreSubsystem* Scoring = PlayUnreal::CreateScoring();

	UE_LOG(LogTemp, Display, TEXT("[PlayUnreal] === Scenario 4: Game Over ==="));

	GM->StartGame();
	GM->OnSpawningComplete();
	TestEqual(TEXT("Playing"), GM->CurrentState, EGameState::Playing);
	PlayUnreal::LogState(this, GM, Scoring, Frog);

	// Simulate losing lives until only 1 remains
	Scoring->LoseLife(); // 3 -> 2
	Scoring->LoseLife(); // 2 -> 1
	TestEqual(TEXT("Lives at 1"), Scoring->Lives, 1);

	// Kill the frog — flag pending game over since this is the last life
	GM->HandleFrogDied(EDeathType::Squish);
	TestEqual(TEXT("Dying"), GM->CurrentState, EGameState::Dying);

	// Simulate scoring subsystem flagging game over
	Scoring->LoseLife(); // 1 -> 0
	TestTrue(TEXT("Scoring says game over"), Scoring->IsGameOver());
	GM->bPendingGameOver = true;

	// Complete dying → should go to GameOver (not Spawning)
	GM->OnDyingComplete();
	TestEqual(TEXT("GameOver state"), GM->CurrentState, EGameState::GameOver);
	PlayUnreal::LogState(this, GM, Scoring, Frog);

	// Verify recovery path: ReturnToTitle → StartGame
	GM->ReturnToTitle();
	TestEqual(TEXT("Back to Title"), GM->CurrentState, EGameState::Title);

	GM->StartGame();
	GM->OnSpawningComplete();
	TestEqual(TEXT("Can play again"), GM->CurrentState, EGameState::Playing);
	TestFalse(TEXT("bPendingGameOver reset"), GM->bPendingGameOver);

	PlayUnreal::LogState(this, GM, Scoring, Frog);
	UE_LOG(LogTemp, Display, TEXT("[PlayUnreal] === Scenario 4: PASSED ==="));

	return true;
}

// ===========================================================================
// Scenario 5: Full Round
// Fill all 5 home slots, verify RoundComplete state, wave increments, bonus.
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayUnreal_FullRound,
	"UnrealFrog.PlayUnreal.Scenario5_FullRound",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPlayUnreal_FullRound::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	UScoreSubsystem* Scoring = PlayUnreal::CreateScoring();

	UE_LOG(LogTemp, Display, TEXT("[PlayUnreal] === Scenario 5: Full Round ==="));

	GM->StartGame();
	GM->OnSpawningComplete();
	TestEqual(TEXT("Playing"), GM->CurrentState, EGameState::Playing);
	TestEqual(TEXT("Wave 1"), GM->CurrentWave, 1);
	PlayUnreal::LogState(this, GM, Scoring, Frog);

	// Fill home slots 1 through 4 (non-final fills → Spawning → Playing cycle)
	const int32 HomeSlotCols[] = {1, 4, 6, 8, 11};

	for (int32 i = 0; i < 4; i++)
	{
		GM->HandleHopCompleted(FIntPoint(HomeSlotCols[i], 14));
		TestEqual(
			*FString::Printf(TEXT("Home slot %d filled"), i + 1),
			GM->HomeSlotsFilledCount, i + 1);
		TestEqual(TEXT("Spawning after non-final slot fill"),
			GM->CurrentState, EGameState::Spawning);

		GM->OnSpawningComplete();
		TestEqual(TEXT("Playing after respawn"),
			GM->CurrentState, EGameState::Playing);
	}

	// Fill the 5th (final) home slot → RoundComplete
	GM->HandleHopCompleted(FIntPoint(HomeSlotCols[4], 14));
	TestEqual(TEXT("All 5 slots filled"), GM->HomeSlotsFilledCount, 5);
	TestEqual(TEXT("RoundComplete state"), GM->CurrentState, EGameState::RoundComplete);
	PlayUnreal::LogState(this, GM, Scoring, Frog);

	// Complete the round
	GM->OnRoundCompleteFinished();
	TestEqual(TEXT("Wave incremented to 2"), GM->CurrentWave, 2);
	TestEqual(TEXT("Spawning for wave 2"), GM->CurrentState, EGameState::Spawning);
	TestEqual(TEXT("Home slots reset"), GM->HomeSlotsFilledCount, 0);
	TestEqual(TEXT("Timer reset"), GM->RemainingTime, GM->TimePerLevel);

	// Can start playing wave 2
	GM->OnSpawningComplete();
	TestEqual(TEXT("Playing wave 2"), GM->CurrentState, EGameState::Playing);

	PlayUnreal::LogState(this, GM, Scoring, Frog);
	UE_LOG(LogTemp, Display, TEXT("[PlayUnreal] === Scenario 5: PASSED ==="));

	return true;
}

// ===========================================================================
// Scenario 6: GetGameStateJSON
// Verify the JSON helper returns valid, parseable state.
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayUnreal_GetGameStateJSON,
	"UnrealFrog.PlayUnreal.Scenario6_GetGameStateJSON",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPlayUnreal_GetGameStateJSON::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	UE_LOG(LogTemp, Display, TEXT("[PlayUnreal] === Scenario 6: GetGameStateJSON ==="));

	// Title state — no player pawn or scoring subsystem in test context
	FString JSON = GM->GetGameStateJSON();
	UE_LOG(LogTemp, Display, TEXT("[PlayUnreal] JSON=%s"), *JSON);

	// Verify JSON contains all required fields
	TestTrue(TEXT("Has score field"), JSON.Contains(TEXT("\"score\":")));
	TestTrue(TEXT("Has lives field"), JSON.Contains(TEXT("\"lives\":")));
	TestTrue(TEXT("Has wave field"), JSON.Contains(TEXT("\"wave\":")));
	TestTrue(TEXT("Has frogPos field"), JSON.Contains(TEXT("\"frogPos\":")));
	TestTrue(TEXT("Has gameState field"), JSON.Contains(TEXT("\"gameState\":")));
	TestTrue(TEXT("Has timeRemaining field"), JSON.Contains(TEXT("\"timeRemaining\":")));
	TestTrue(TEXT("Has homeSlotsFilledCount field"), JSON.Contains(TEXT("\"homeSlotsFilledCount\":")));

	// Verify state name is correct for Title
	TestTrue(TEXT("State is Title"), JSON.Contains(TEXT("\"gameState\":\"Title\"")));

	// Start game and verify state transitions reflect in JSON
	GM->StartGame();
	GM->OnSpawningComplete();
	FString PlayingJSON = GM->GetGameStateJSON();
	TestTrue(TEXT("State is Playing after start"), PlayingJSON.Contains(TEXT("\"gameState\":\"Playing\"")));

	// Verify wave and home slots
	TestTrue(TEXT("Wave is 1"), PlayingJSON.Contains(TEXT("\"wave\":1")));
	TestTrue(TEXT("HomeSlotsFilledCount is 0"), PlayingJSON.Contains(TEXT("\"homeSlotsFilledCount\":0")));

	UE_LOG(LogTemp, Display, TEXT("[PlayUnreal] PlayingJSON=%s"), *PlayingJSON);
	UE_LOG(LogTemp, Display, TEXT("[PlayUnreal] === Scenario 6: PASSED ==="));

	return true;
}

#endif // WITH_AUTOMATION_TESTS
