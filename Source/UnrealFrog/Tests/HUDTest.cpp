// Copyright UnrealFrog Team. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Core/FroggerHUD.h"

#if WITH_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Test: Default display values
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUD_DefaultValues,
	"UnrealFrog.HUD.DefaultValues",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHUD_DefaultValues::RunTest(const FString& Parameters)
{
	AFroggerHUD* HUD = NewObject<AFroggerHUD>();

	TestEqual(TEXT("Score starts at 0"), HUD->DisplayScore, 0);
	TestEqual(TEXT("HighScore starts at 0"), HUD->DisplayHighScore, 0);
	TestEqual(TEXT("Lives starts at 3"), HUD->DisplayLives, 3);
	TestEqual(TEXT("Wave starts at 1"), HUD->DisplayWave, 1);
	TestNearlyEqual(TEXT("Timer percent starts at 1.0"), HUD->TimerPercent, 1.0f);
	TestEqual(TEXT("State starts at Title"), HUD->DisplayState, EGameState::Title);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Score and lives update
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUD_ScoreUpdate,
	"UnrealFrog.HUD.ScoreUpdate",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHUD_ScoreUpdate::RunTest(const FString& Parameters)
{
	AFroggerHUD* HUD = NewObject<AFroggerHUD>();

	HUD->UpdateScore(1500);
	TestEqual(TEXT("Score updated"), HUD->DisplayScore, 1500);

	HUD->UpdateLives(5);
	TestEqual(TEXT("Lives updated"), HUD->DisplayLives, 5);

	HUD->UpdateWave(3);
	TestEqual(TEXT("Wave updated"), HUD->DisplayWave, 3);

	HUD->UpdateHighScore(9999);
	TestEqual(TEXT("High score updated"), HUD->DisplayHighScore, 9999);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Timer percent calculation
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUD_TimerPercent,
	"UnrealFrog.HUD.TimerPercent",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHUD_TimerPercent::RunTest(const FString& Parameters)
{
	AFroggerHUD* HUD = NewObject<AFroggerHUD>();

	HUD->UpdateTimer(15.0f, 30.0f);
	TestNearlyEqual(TEXT("50% time remaining"), HUD->TimerPercent, 0.5f);

	HUD->UpdateTimer(0.0f, 30.0f);
	TestNearlyEqual(TEXT("0% time remaining"), HUD->TimerPercent, 0.0f);

	HUD->UpdateTimer(30.0f, 30.0f);
	TestNearlyEqual(TEXT("100% time remaining"), HUD->TimerPercent, 1.0f);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Overlay text for each game state
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUD_OverlayText,
	"UnrealFrog.HUD.OverlayText",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHUD_OverlayText::RunTest(const FString& Parameters)
{
	AFroggerHUD* HUD = NewObject<AFroggerHUD>();

	HUD->UpdateGameState(EGameState::Title);
	TestEqual(TEXT("Title overlay"), HUD->GetOverlayText(), FString(TEXT("PRESS START")));

	HUD->UpdateGameState(EGameState::GameOver);
	TestEqual(TEXT("GameOver overlay"), HUD->GetOverlayText(), FString(TEXT("GAME OVER")));

	HUD->UpdateGameState(EGameState::RoundComplete);
	TestEqual(TEXT("RoundComplete overlay"), HUD->GetOverlayText(), FString(TEXT("ROUND COMPLETE")));

	HUD->UpdateGameState(EGameState::Playing);
	TestEqual(TEXT("Playing has no overlay"), HUD->GetOverlayText(), FString(TEXT("")));

	HUD->UpdateGameState(EGameState::Dying);
	TestTrue(TEXT("Dying has overlay text"), HUD->GetOverlayText().Len() > 0);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
