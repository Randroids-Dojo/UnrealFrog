// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/ScoreSubsystem.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

void UScoreSubsystem::AddForwardHopScore()
{
	int32 Points = FMath::RoundToInt(static_cast<float>(PointsPerHop) * Multiplier);
	Score += Points;

	// Increase multiplier for the next consecutive forward hop, capped at MaxMultiplier
	Multiplier += MultiplierIncrement;
	Multiplier = FMath::Min(Multiplier, MaxMultiplier);

	NotifyScoreChanged();
	CheckExtraLife();
}

void UScoreSubsystem::AddTimeBonus(float RemainingSeconds)
{
	if (RemainingSeconds <= 0.0f)
	{
		return;
	}

	int32 Bonus = FMath::FloorToInt(RemainingSeconds) * 10;
	Score += Bonus;

	NotifyScoreChanged();
	CheckExtraLife();
}

void UScoreSubsystem::AddBonusPoints(int32 Points)
{
	Score += Points;

	NotifyScoreChanged();
	CheckExtraLife();
}

void UScoreSubsystem::AddHomeSlotScore()
{
	AddBonusPoints(HomeSlotPoints);
}

void UScoreSubsystem::ResetMultiplier()
{
	Multiplier = 1.0f;
}

void UScoreSubsystem::LoseLife()
{
	ResetMultiplier();

	if (Lives > 0)
	{
		Lives--;
		OnLivesChanged.Broadcast(Lives);
	}

	if (Lives == 0)
	{
		OnGameOver.Broadcast(Score);
	}
}

void UScoreSubsystem::StartNewGame()
{
	LoadHighScore();
	Score = 0;
	Lives = InitialLives;
	Multiplier = 1.0f;
	LastExtraLifeThreshold = 0;

	OnScoreChanged.Broadcast(Score);
	OnLivesChanged.Broadcast(Lives);
}

bool UScoreSubsystem::IsGameOver() const
{
	return Lives <= 0;
}

void UScoreSubsystem::NotifyScoreChanged()
{
	if (Score > HighScore)
	{
		HighScore = Score;
		SaveHighScore();
	}

	OnScoreChanged.Broadcast(Score);
}

void UScoreSubsystem::CheckExtraLife()
{
	// Determine which threshold bracket the score is in
	int32 CurrentBracket = Score / ExtraLifeThreshold;
	int32 PreviousBracket = LastExtraLifeThreshold / ExtraLifeThreshold;

	if (CurrentBracket > PreviousBracket && Lives < MaxLives)
	{
		Lives++;
		LastExtraLifeThreshold = CurrentBracket * ExtraLifeThreshold;

		OnExtraLife.Broadcast();
		OnLivesChanged.Broadcast(Lives);
	}
	else if (CurrentBracket > PreviousBracket)
	{
		// Track the threshold even if lives are capped, so we don't re-award later
		LastExtraLifeThreshold = CurrentBracket * ExtraLifeThreshold;
	}
}

void UScoreSubsystem::LoadHighScore()
{
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("highscore.txt");
	FString FileContent;
	if (FFileHelper::LoadFileToString(FileContent, *FilePath))
	{
		int32 LoadedScore = FCString::Atoi(*FileContent);
		if (LoadedScore > HighScore)
		{
			HighScore = LoadedScore;
		}
	}
}

void UScoreSubsystem::SaveHighScore()
{
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("highscore.txt");
	FFileHelper::SaveStringToFile(FString::FromInt(HighScore), *FilePath);
}
