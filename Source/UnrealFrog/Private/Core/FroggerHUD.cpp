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

	DrawScorePanel();
	DrawTimerBar();
	DrawOverlay();
}

void AFroggerHUD::DrawScorePanel()
{
	float ScreenW = Canvas->SizeX;

	// Top-left: Score
	FString ScoreStr = FString::Printf(TEXT("SCORE: %05d"), DisplayScore);
	Canvas->SetDrawColor(FColor::White);
	Canvas->DrawText(GEngine->GetMediumFont(), ScoreStr, 20.0f, 10.0f);

	// Top-center: High Score
	FString HiStr = FString::Printf(TEXT("HI: %05d"), DisplayHighScore);
	float HiWidth = 0.0f;
	float HiHeight = 0.0f;
	Canvas->TextSize(GEngine->GetMediumFont(), HiStr, HiWidth, HiHeight);
	Canvas->DrawText(GEngine->GetMediumFont(), HiStr, (ScreenW - HiWidth) * 0.5f, 10.0f);

	// Top-right: Lives and Wave
	FString LivesStr = FString::Printf(TEXT("LIVES: %d  WAVE: %d"), DisplayLives, DisplayWave);
	float LivesWidth = 0.0f;
	float LivesHeight = 0.0f;
	Canvas->TextSize(GEngine->GetMediumFont(), LivesStr, LivesWidth, LivesHeight);
	Canvas->DrawText(GEngine->GetMediumFont(), LivesStr, ScreenW - LivesWidth - 20.0f, 10.0f);
}

void AFroggerHUD::DrawTimerBar()
{
	float ScreenW = Canvas->SizeX;

	// Timer bar across the top, just below the score text
	float BarY = 40.0f;
	float BarH = 8.0f;
	float BarMaxW = ScreenW - 40.0f;

	// Background (dark)
	Canvas->SetDrawColor(FColor(40, 40, 40, 200));
	FCanvasTileItem BgTile(FVector2D(20.0f, BarY), FVector2D(BarMaxW, BarH), FColor(40, 40, 40, 200));
	Canvas->DrawItem(BgTile);

	// Foreground (green→red based on percent)
	float FilledW = BarMaxW * TimerPercent;
	if (FilledW > 0.0f)
	{
		uint8 R = static_cast<uint8>((1.0f - TimerPercent) * 255.0f);
		uint8 G = static_cast<uint8>(TimerPercent * 255.0f);
		FColor BarColor(R, G, 0, 255);
		FCanvasTileItem FillTile(FVector2D(20.0f, BarY), FVector2D(FilledW, BarH), BarColor);
		Canvas->DrawItem(FillTile);
	}
}

void AFroggerHUD::DrawOverlay()
{
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
