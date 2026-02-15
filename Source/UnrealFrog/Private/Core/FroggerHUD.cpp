// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/FroggerHUD.h"
#include "Core/ScoreSubsystem.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

AFroggerHUD::AFroggerHUD()
{
}

// -- Update methods -----------------------------------------------------------

void AFroggerHUD::UpdateScore(int32 NewScore)
{
	DisplayScore = NewScore;
}

void AFroggerHUD::UpdateHighScore(int32 NewHighScore)
{
	DisplayHighScore = NewHighScore;
}

void AFroggerHUD::UpdateLives(int32 NewLives)
{
	DisplayLives = NewLives;
}

void AFroggerHUD::UpdateWave(int32 Wave)
{
	DisplayWave = Wave;
}

void AFroggerHUD::UpdateTimer(float TimeRemaining, float MaxTime)
{
	if (MaxTime > 0.0f)
	{
		TimerPercent = FMath::Clamp(TimeRemaining / MaxTime, 0.0f, 1.0f);
	}
	else
	{
		TimerPercent = 0.0f;
	}
}

void AFroggerHUD::UpdateGameState(EGameState NewState)
{
	DisplayState = NewState;
}

FString AFroggerHUD::GetOverlayText() const
{
	switch (DisplayState)
	{
	case EGameState::Title:
		return TEXT("PRESS START");
	case EGameState::Paused:
		return TEXT("PAUSED");
	case EGameState::GameOver:
		return TEXT("GAME OVER");
	case EGameState::RoundComplete:
		return TEXT("ROUND COMPLETE");
	case EGameState::Spawning:
		return TEXT("GET READY");
	case EGameState::Dying:
		return TEXT("OOPS!");
	default:
		return TEXT("");
	}
}

// -- Drawing ------------------------------------------------------------------

void AFroggerHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	// Sync display state from GameMode and ScoreSubsystem each frame
	if (UWorld* World = GetWorld())
	{
		if (AUnrealFrogGameMode* GM = Cast<AUnrealFrogGameMode>(World->GetAuthGameMode()))
		{
			DisplayState = GM->CurrentState;
			DisplayWave = GM->CurrentWave;
			if (GM->TimePerLevel > 0.0f)
			{
				TimerPercent = FMath::Clamp(GM->RemainingTime / GM->TimePerLevel, 0.0f, 1.0f);
			}
		}

		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UScoreSubsystem* Scoring = GI->GetSubsystem<UScoreSubsystem>())
			{
				DisplayScore = Scoring->Score;
				DisplayHighScore = Scoring->HighScore;
				DisplayLives = Scoring->Lives;
			}
		}
	}

	// Score pop detection (polling approach — no new delegate wiring needed)
	if (DisplayScore > PreviousScore)
	{
		int32 Delta = DisplayScore - PreviousScore;

		FScorePop Pop;
		Pop.Text = FString::Printf(TEXT("+%d"), Delta);
		// Position proportionally after the score text
		FString ScoreText = FString::Printf(TEXT("SCORE: %05d"), DisplayScore);
		Pop.Position = FVector2D(20.0f + ScoreText.Len() * 10.0f, 10.0f);
		Pop.SpawnTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		Pop.Duration = 1.5f;
		Pop.Color = (Delta > 100) ? FColor::White : FColor::Yellow;
		ActiveScorePops.Add(Pop);

		PreviousScore = DisplayScore;
	}
	else if (DisplayScore < PreviousScore)
	{
		// Score reset (new game)
		PreviousScore = DisplayScore;
	}

	// Wave announcement detection
	if (DisplayWave > PreviousWave && DisplayWave > 1)
	{
		WaveAnnounceText = FString::Printf(TEXT("WAVE %d!"), DisplayWave);
		WaveAnnounceStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		bShowingWaveAnnounce = true;
		PreviousWave = DisplayWave;
	}
	else if (DisplayWave < PreviousWave)
	{
		// Wave reset (new game)
		PreviousWave = DisplayWave;
		bShowingWaveAnnounce = false;
	}

	// Timer pulse detection
	bTimerPulsing = (TimerPercent < 0.167f && TimerPercent > 0.0f
		&& DisplayState == EGameState::Playing);

	// Title state gets a full title screen
	if (DisplayState == EGameState::Title)
	{
		DrawTitleScreen();
		return;
	}

	// Death flash: trigger on Dying state, decay over time
	if (DisplayState == EGameState::Dying && DeathFlashAlpha <= 0.0f)
	{
		DeathFlashAlpha = 0.5f;
	}
	if (DeathFlashAlpha > 0.0f)
	{
		DrawDeathFlash();
		// Decay at ~1.67/s -> 0.5 alpha decays to 0 in ~0.3s
		float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
		DeathFlashAlpha = FMath::Max(0.0f, DeathFlashAlpha - DeltaTime * 1.67f);
	}

	DrawScorePanel();
	DrawTimerBar();
	DrawScorePops();
	DrawWaveAnnouncement();
	DrawOverlay();
}

void AFroggerHUD::DrawScorePanel()
{
	float ScreenW = Canvas->SizeX;

	// Top-left: Score (arcade green)
	FString ScoreStr = FString::Printf(TEXT("SCORE: %05d"), DisplayScore);
	Canvas->SetDrawColor(FColor(0, 255, 100, 255));
	Canvas->DrawText(GEngine->GetMediumFont(), ScoreStr, 20.0f, 10.0f);

	// Top-center: High Score (yellow)
	FString HiStr = FString::Printf(TEXT("HI: %05d"), DisplayHighScore);
	float HiWidth = 0.0f;
	float HiHeight = 0.0f;
	Canvas->TextSize(GEngine->GetMediumFont(), HiStr, HiWidth, HiHeight);
	Canvas->SetDrawColor(FColor(255, 255, 0, 255));
	Canvas->DrawText(GEngine->GetMediumFont(), HiStr, (ScreenW - HiWidth) * 0.5f, 10.0f);

	// Top-right: Lives (red-tinted) and Wave
	FString LivesStr = FString::Printf(TEXT("LIVES: %d  WAVE: %d"), DisplayLives, DisplayWave);
	float LivesWidth = 0.0f;
	float LivesHeight = 0.0f;
	Canvas->TextSize(GEngine->GetMediumFont(), LivesStr, LivesWidth, LivesHeight);
	Canvas->SetDrawColor(FColor(255, 100, 100, 255));
	Canvas->DrawText(GEngine->GetMediumFont(), LivesStr, ScreenW - LivesWidth - 20.0f, 10.0f);
}

void AFroggerHUD::DrawTimerBar()
{
	float ScreenW = Canvas->SizeX;

	// Timer bar across the top, just below the score text
	float BarY = 40.0f;
	float BarH = 8.0f;
	float BarMaxW = ScreenW - 40.0f;

	// Timer pulse effect: oscillate bar height when pulsing
	if (bTimerPulsing)
	{
		float Time = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		float Pulse = FMath::Abs(FMath::Sin(Time * 6.0f)); // ~1Hz oscillation
		BarH = 8.0f + Pulse * 6.0f; // 8-14px height
		TimerPulseAlpha = Pulse;
	}
	else
	{
		TimerPulseAlpha = 0.0f;
	}

	// Background (near-black)
	FColor BgColor(20, 20, 20, 220);
	FCanvasTileItem BgTile(FVector2D(20.0f, BarY), FVector2D(BarMaxW, BarH), BgColor);
	Canvas->DrawItem(BgTile);

	// Foreground (green->red based on percent, flash red when pulsing)
	float FilledW = BarMaxW * TimerPercent;
	if (FilledW > 0.0f)
	{
		FColor BarColor;
		if (bTimerPulsing)
		{
			// Flash between red and dark red
			uint8 FlashR = static_cast<uint8>(180.0f + TimerPulseAlpha * 75.0f);
			BarColor = FColor(FlashR, 0, 0, 255);
		}
		else
		{
			uint8 R = static_cast<uint8>((1.0f - TimerPercent) * 255.0f);
			uint8 G = static_cast<uint8>(TimerPercent * 255.0f);
			BarColor = FColor(R, G, 0, 255);
		}
		FCanvasTileItem FillTile(FVector2D(20.0f, BarY), FVector2D(FilledW, BarH), BarColor);
		Canvas->DrawItem(FillTile);
	}
}

void AFroggerHUD::DrawScorePops()
{
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	for (int32 i = ActiveScorePops.Num() - 1; i >= 0; --i)
	{
		FScorePop& Pop = ActiveScorePops[i];
		float Elapsed = CurrentTime - Pop.SpawnTime;
		float Alpha = FMath::Clamp(Elapsed / Pop.Duration, 0.0f, 1.0f);

		if (Alpha >= 1.0f)
		{
			ActiveScorePops.RemoveAt(i);
			continue;
		}

		// Rise upward 60px over duration
		float YOffset = Alpha * 60.0f;
		// Alpha fade
		uint8 TextAlpha = static_cast<uint8>((1.0f - Alpha) * 255.0f);

		FColor DrawColor = Pop.Color;
		DrawColor.A = TextAlpha;

		Canvas->SetDrawColor(DrawColor);
		Canvas->DrawText(GEngine->GetMediumFont(), Pop.Text,
			Pop.Position.X, Pop.Position.Y - YOffset);
	}
}

void AFroggerHUD::DrawWaveAnnouncement()
{
	if (!bShowingWaveAnnounce)
	{
		return;
	}

	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	float Elapsed = CurrentTime - WaveAnnounceStartTime;
	float TotalDuration = 2.0f; // 0.3s fade in + 1.0s hold + 0.7s fade out

	if (Elapsed >= TotalDuration)
	{
		bShowingWaveAnnounce = false;
		return;
	}

	// Calculate alpha: fade in 0.3s, hold 1.0s, fade out 0.7s
	float TextAlpha;
	if (Elapsed < 0.3f)
	{
		TextAlpha = Elapsed / 0.3f;
	}
	else if (Elapsed < 1.3f)
	{
		TextAlpha = 1.0f;
	}
	else
	{
		TextAlpha = 1.0f - (Elapsed - 1.3f) / 0.7f;
	}

	uint8 AlphaByte = static_cast<uint8>(FMath::Clamp(TextAlpha, 0.0f, 1.0f) * 255.0f);

	float ScreenW = Canvas->SizeX;
	float ScreenH = Canvas->SizeY;
	UFont* Font = GEngine->GetLargeFont();

	float TextW = 0.0f;
	float TextH = 0.0f;
	Canvas->TextSize(Font, WaveAnnounceText, TextW, TextH);

	Canvas->SetDrawColor(FColor(255, 255, 0, AlphaByte));
	Canvas->DrawText(Font, WaveAnnounceText,
		(ScreenW - TextW) * 0.5f, ScreenH * 0.3f);
}

void AFroggerHUD::DrawTitleScreen()
{
	float ScreenW = Canvas->SizeX;
	float ScreenH = Canvas->SizeY;
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// Full-screen dark overlay background
	FCanvasTileItem BgTile(
		FVector2D(0.0f, 0.0f),
		FVector2D(ScreenW, ScreenH),
		FColor(10, 10, 20, 240));
	Canvas->DrawItem(BgTile);

	UFont* LargeFont = GEngine->GetLargeFont();
	UFont* MediumFont = GEngine->GetMediumFont();

	// 1. "UNREAL FROG" -- large, centered at ~30% height, color pulse green<->yellow
	{
		FString TitleText = TEXT("UNREAL FROG");
		float TitleW = 0.0f;
		float TitleH = 0.0f;
		Canvas->TextSize(LargeFont, TitleText, TitleW, TitleH);

		// Color pulse: green (0,255,100) <-> yellow (255,255,0) at 1.5Hz
		float Pulse = (FMath::Sin(CurrentTime * 1.5f * 2.0f * PI) + 1.0f) * 0.5f;
		uint8 R = static_cast<uint8>(Pulse * 255.0f);
		uint8 G = 255;
		uint8 B = static_cast<uint8>((1.0f - Pulse) * 100.0f);
		Canvas->SetDrawColor(FColor(R, G, B, 255));
		Canvas->DrawText(LargeFont, TitleText,
			(ScreenW - TitleW) * 0.5f, ScreenH * 0.3f);
	}

	// 2. "PRESS START" -- blinks on/off at 2Hz
	{
		float Blink = FMath::Sin(CurrentTime * 2.0f * 2.0f * PI);
		if (Blink > 0.0f)
		{
			FString StartText = TEXT("PRESS START");
			float StartW = 0.0f;
			float StartH = 0.0f;
			Canvas->TextSize(MediumFont, StartText, StartW, StartH);

			Canvas->SetDrawColor(FColor::White);
			Canvas->DrawText(MediumFont, StartText,
				(ScreenW - StartW) * 0.5f, ScreenH * 0.55f);
		}
	}

	// 3. "HI-SCORE: {N}" -- yellow, only shown if > 0
	if (DisplayHighScore > 0)
	{
		FString HiStr = FString::Printf(TEXT("HI-SCORE: %d"), DisplayHighScore);
		float HiW = 0.0f;
		float HiH = 0.0f;
		Canvas->TextSize(MediumFont, HiStr, HiW, HiH);

		Canvas->SetDrawColor(FColor(255, 255, 0, 255));
		Canvas->DrawText(MediumFont, HiStr,
			(ScreenW - HiW) * 0.5f, ScreenH * 0.45f);
	}

	// 4. Credits line -- dim gray at bottom
	{
		FString Credits = TEXT("A MOB PROGRAMMING PRODUCTION");
		float CredW = 0.0f;
		float CredH = 0.0f;
		Canvas->TextSize(MediumFont, Credits, CredW, CredH);

		Canvas->SetDrawColor(FColor(100, 100, 100, 200));
		Canvas->DrawText(MediumFont, Credits,
			(ScreenW - CredW) * 0.5f, ScreenH * 0.85f);
	}
}

void AFroggerHUD::TickDeathFlash(float DeltaTime)
{
	if (DeathFlashAlpha > 0.0f)
	{
		DeathFlashAlpha = FMath::Max(0.0f, DeathFlashAlpha - DeltaTime * 1.67f);
	}
}

void AFroggerHUD::DrawDeathFlash()
{
	if (!Canvas || DeathFlashAlpha <= 0.0f)
	{
		return;
	}

	float ScreenW = Canvas->SizeX;
	float ScreenH = Canvas->SizeY;

	uint8 Alpha = static_cast<uint8>(DeathFlashAlpha * 255.0f);
	FCanvasTileItem FlashTile(
		FVector2D(0.0f, 0.0f),
		FVector2D(ScreenW, ScreenH),
		FColor(255, 0, 0, Alpha));
	FlashTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(FlashTile);
}

void AFroggerHUD::DrawOverlay()
{
	// Title state is handled by DrawTitleScreen, skip default overlay
	if (DisplayState == EGameState::Title)
	{
		return;
	}

	FString Overlay = GetOverlayText();
	if (Overlay.IsEmpty())
	{
		return;
	}

	float ScreenW = Canvas->SizeX;
	float ScreenH = Canvas->SizeY;

	// Draw centered overlay text with large font
	UFont* Font = GEngine->GetLargeFont();

	float TextW = 0.0f;
	float TextH = 0.0f;
	Canvas->TextSize(Font, Overlay, TextW, TextH);

	// Semi-transparent background
	float PadX = 40.0f;
	float PadY = 20.0f;
	float BgX = (ScreenW - TextW) * 0.5f - PadX;
	float BgY = (ScreenH - TextH) * 0.5f - PadY;
	FCanvasTileItem BgTile(
		FVector2D(BgX, BgY),
		FVector2D(TextW + PadX * 2.0f, TextH + PadY * 2.0f),
		FColor(0, 0, 0, 180));
	Canvas->DrawItem(BgTile);

	// Overlay text in yellow
	Canvas->SetDrawColor(FColor::Yellow);
	Canvas->DrawText(Font, Overlay, (ScreenW - TextW) * 0.5f, (ScreenH - TextH) * 0.5f);
}
