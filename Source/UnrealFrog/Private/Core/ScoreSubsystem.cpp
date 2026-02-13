// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/ScoreSubsystem.h"

void UScoreSubsystem::AddForwardHopScore()
{
	int32 Points = FMath::RoundToInt(static_cast<float>(PointsPerHop) * Multiplier);
	Score += Points;

	// Increase multiplier for the next consecutive forward hop
	Multiplier += MultiplierIncrement;

	NotifyScoreChanged();
	CheckExtraLife();
}

void UScoreSubsystem::AddTimeBonus(float RemainingTime, float MaxTime)
{
	if (MaxTime <= 0.0f)
	{
		return;
	}

	float Ratio = FMath::Clamp(RemainingTime / MaxTime, 0.0f, 1.0f);
	int32 Bonus = FMath::RoundToInt(Ratio * 1000.0f);
	Score += Bonus;

	NotifyScoreChanged();
	CheckExtraLife();
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
