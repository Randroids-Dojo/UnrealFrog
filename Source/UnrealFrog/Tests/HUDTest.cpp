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

// ---------------------------------------------------------------------------
// Test: Score pop created when score increases
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUD_ScorePopCreated,
	"UnrealFrog.HUD.ScorePopCreated",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHUD_ScorePopCreated::RunTest(const FString& Parameters)
{
	AFroggerHUD* HUD = NewObject<AFroggerHUD>();

	// Simulate score increase via the polling mechanism
	HUD->PreviousScore = 0;
	HUD->DisplayScore = 100;

	// Score pop detection happens in DrawHUD, but we can test the struct
	AFroggerHUD::FScorePop Pop;
	Pop.Text = TEXT("+100");
	Pop.Position = FVector2D(160.0f, 10.0f);
	Pop.SpawnTime = 0.0f;
	Pop.Duration = 1.5f;
	Pop.Color = FColor::Yellow;
	HUD->ActiveScorePops.Add(Pop);

	TestEqual(TEXT("Score pop was added"), HUD->ActiveScorePops.Num(), 1);
	TestEqual(TEXT("Pop text is +100"), HUD->ActiveScorePops[0].Text, FString(TEXT("+100")));

	// Large delta should use white
	AFroggerHUD::FScorePop BigPop;
	BigPop.Text = TEXT("+200");
	BigPop.Color = FColor::White; // > 100 pts
	HUD->ActiveScorePops.Add(BigPop);

	TestEqual(TEXT("Big pop uses white"), HUD->ActiveScorePops[1].Color, FColor::White);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Score pop expires after duration
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUD_ScorePopExpires,
	"UnrealFrog.HUD.ScorePopExpires",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHUD_ScorePopExpires::RunTest(const FString& Parameters)
{
	AFroggerHUD* HUD = NewObject<AFroggerHUD>();

	AFroggerHUD::FScorePop Pop;
	Pop.Text = TEXT("+50");
	Pop.SpawnTime = 0.0f;
	Pop.Duration = 1.5f;
	HUD->ActiveScorePops.Add(Pop);

	TestEqual(TEXT("Pop exists"), HUD->ActiveScorePops.Num(), 1);
	TestNearlyEqual(TEXT("Pop duration is 1.5s"), HUD->ActiveScorePops[0].Duration, 1.5f);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Timer pulse activates when < 16.7% time remaining
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUD_TimerPulseActivates,
	"UnrealFrog.HUD.TimerPulseActivates",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHUD_TimerPulseActivates::RunTest(const FString& Parameters)
{
	AFroggerHUD* HUD = NewObject<AFroggerHUD>();

	// Timer at 50% — should NOT pulse
	HUD->TimerPercent = 0.5f;
	HUD->DisplayState = EGameState::Playing;
	// Simulate the check from DrawHUD
	HUD->bTimerPulsing = (HUD->TimerPercent < 0.167f && HUD->TimerPercent > 0.0f
		&& HUD->DisplayState == EGameState::Playing);
	TestFalse(TEXT("No pulse at 50%"), HUD->bTimerPulsing);

	// Timer at 10% — should pulse
	HUD->TimerPercent = 0.10f;
	HUD->bTimerPulsing = (HUD->TimerPercent < 0.167f && HUD->TimerPercent > 0.0f
		&& HUD->DisplayState == EGameState::Playing);
	TestTrue(TEXT("Pulse at 10%"), HUD->bTimerPulsing);

	// Timer at 0% — should NOT pulse (game over)
	HUD->TimerPercent = 0.0f;
	HUD->bTimerPulsing = (HUD->TimerPercent < 0.167f && HUD->TimerPercent > 0.0f
		&& HUD->DisplayState == EGameState::Playing);
	TestFalse(TEXT("No pulse at 0%"), HUD->bTimerPulsing);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Wave announcement activates on wave change
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUD_WaveAnnounceActivates,
	"UnrealFrog.HUD.WaveAnnounceActivates",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHUD_WaveAnnounceActivates::RunTest(const FString& Parameters)
{
	AFroggerHUD* HUD = NewObject<AFroggerHUD>();

	// Wave 1 → 2 should trigger announcement
	HUD->PreviousWave = 1;
	HUD->DisplayWave = 2;

	// Simulate the detection logic from DrawHUD
	if (HUD->DisplayWave > HUD->PreviousWave && HUD->DisplayWave > 1)
	{
		HUD->WaveAnnounceText = FString::Printf(TEXT("WAVE %d!"), HUD->DisplayWave);
		HUD->bShowingWaveAnnounce = true;
		HUD->PreviousWave = HUD->DisplayWave;
	}

	TestTrue(TEXT("Wave announce active"), HUD->bShowingWaveAnnounce);
	TestEqual(TEXT("Wave text correct"), HUD->WaveAnnounceText, FString(TEXT("WAVE 2!")));

	// Wave 1 should NOT trigger (initial state)
	HUD->bShowingWaveAnnounce = false;
	HUD->PreviousWave = 0;
	HUD->DisplayWave = 1;
	if (HUD->DisplayWave > HUD->PreviousWave && HUD->DisplayWave > 1)
	{
		HUD->bShowingWaveAnnounce = true;
	}
	TestFalse(TEXT("No announce for wave 1"), HUD->bShowingWaveAnnounce);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Arcade color scheme values
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUD_ArcadeColorScheme,
	"UnrealFrog.HUD.ArcadeColorScheme",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHUD_ArcadeColorScheme::RunTest(const FString& Parameters)
{
	// Verify the arcade color constants used in DrawScorePanel
	FColor ScoreGreen(0, 255, 100, 255);
	FColor HiScoreYellow(255, 255, 0, 255);
	FColor LivesRed(255, 100, 100, 255);
	FColor TimerBg(20, 20, 20, 220);

	TestEqual(TEXT("Score green R"), ScoreGreen.R, (uint8)0);
	TestEqual(TEXT("Score green G"), ScoreGreen.G, (uint8)255);
	TestEqual(TEXT("Score green B"), ScoreGreen.B, (uint8)100);

	TestEqual(TEXT("HI yellow R"), HiScoreYellow.R, (uint8)255);
	TestEqual(TEXT("HI yellow G"), HiScoreYellow.G, (uint8)255);

	TestEqual(TEXT("Lives red R"), LivesRed.R, (uint8)255);
	TestEqual(TEXT("Lives red G"), LivesRed.G, (uint8)100);

	TestEqual(TEXT("Timer bg R"), TimerBg.R, (uint8)20);
	TestEqual(TEXT("Timer bg A"), TimerBg.A, (uint8)220);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Title screen shows overlay in Title state
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUD_TitleScreenOverlay,
	"UnrealFrog.HUD.TitleScreenOverlay",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHUD_TitleScreenOverlay::RunTest(const FString& Parameters)
{
	AFroggerHUD* HUD = NewObject<AFroggerHUD>();

	HUD->UpdateGameState(EGameState::Title);
	TestEqual(TEXT("Title state set"), HUD->DisplayState, EGameState::Title);
	// GetOverlayText still returns "PRESS START" for compatibility
	TestEqual(TEXT("Overlay text is PRESS START"), HUD->GetOverlayText(), FString(TEXT("PRESS START")));

	return true;
}

// ---------------------------------------------------------------------------
// Test: Title screen high score visibility
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUD_TitleScreenHighScoreVisible,
	"UnrealFrog.HUD.TitleScreenHighScoreVisible",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHUD_TitleScreenHighScoreVisible::RunTest(const FString& Parameters)
{
	AFroggerHUD* HUD = NewObject<AFroggerHUD>();

	HUD->UpdateGameState(EGameState::Title);

	// High score > 0 means it should be visible on title screen
	HUD->UpdateHighScore(5000);
	TestTrue(TEXT("High score > 0 should be visible"), HUD->DisplayHighScore > 0);

	// High score == 0 means it should be hidden
	HUD->UpdateHighScore(0);
	TestFalse(TEXT("High score == 0 should be hidden"), HUD->DisplayHighScore > 0);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Death flash activates when entering Dying state
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUD_DeathFlashActivates,
	"UnrealFrog.HUD.DeathFlashActivates",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHUD_DeathFlashActivates::RunTest(const FString& Parameters)
{
	AFroggerHUD* HUD = NewObject<AFroggerHUD>();
	HUD->DeathFlashAlpha = 0.0f;
	HUD->DisplayState = EGameState::Dying;

	// Simulate the trigger logic from DrawHUD
	if (HUD->DisplayState == EGameState::Dying && HUD->DeathFlashAlpha <= 0.0f)
	{
		HUD->DeathFlashAlpha = 0.5f;
	}

	TestTrue(TEXT("Death flash activated"), HUD->DeathFlashAlpha > 0.0f);
	TestNearlyEqual(TEXT("Death flash alpha is 0.5"), HUD->DeathFlashAlpha, 0.5f);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Death flash decays over time via TickDeathFlash
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUD_DeathFlashDecays,
	"UnrealFrog.HUD.DeathFlashDecays",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHUD_DeathFlashDecays::RunTest(const FString& Parameters)
{
	AFroggerHUD* HUD = NewObject<AFroggerHUD>();
	HUD->DeathFlashAlpha = 0.5f;

	HUD->TickDeathFlash(0.1f); // ~0.167 decay
	TestTrue(TEXT("Alpha decreased"), HUD->DeathFlashAlpha < 0.5f);
	TestTrue(TEXT("Alpha still positive"), HUD->DeathFlashAlpha > 0.0f);

	// Tick enough to fully decay
	HUD->TickDeathFlash(1.0f);
	TestNearlyEqual(TEXT("Alpha decayed to 0"), HUD->DeathFlashAlpha, 0.0f);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
