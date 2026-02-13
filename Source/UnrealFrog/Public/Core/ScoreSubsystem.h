// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ScoreSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, int32, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLivesChanged, int32, NewLives);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExtraLife);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameOver, int32, FinalScore);

/**
 * Scoring subsystem. Tracks score, high score, lives, and the consecutive
 * forward-hop multiplier. Pure scoring logic with no dependencies on other
 * gameplay systems -- they call into us, we broadcast results.
 */
UCLASS()
class UNREALFROG_API UScoreSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// -- Tunable parameters (EditAnywhere for designer iteration) ----------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	int32 PointsPerHop = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	float MultiplierIncrement = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	int32 ExtraLifeThreshold = 10000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	int32 MaxLives = 9;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	float MaxMultiplier = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	int32 HomeSlotPoints = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	int32 RoundCompleteBonus = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	int32 InitialLives = 3;

	// -- Runtime state ----------------------------------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scoring|State")
	int32 Score = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scoring|State")
	int32 HighScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scoring|State")
	int32 Lives = 3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scoring|State")
	float Multiplier = 1.0f;

	// -- Delegates --------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Scoring|Events")
	FOnScoreChanged OnScoreChanged;

	UPROPERTY(BlueprintAssignable, Category = "Scoring|Events")
	FOnLivesChanged OnLivesChanged;

	UPROPERTY(BlueprintAssignable, Category = "Scoring|Events")
	FOnExtraLife OnExtraLife;

	UPROPERTY(BlueprintAssignable, Category = "Scoring|Events")
	FOnGameOver OnGameOver;

	// -- Public interface -------------------------------------------------

	/** Award points for a forward hop: PointsPerHop * Multiplier, then increase the multiplier. */
	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void AddForwardHopScore();

	/** Award time bonus: floor(RemainingSeconds) * 10 points. */
	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void AddTimeBonus(float RemainingSeconds);

	/** Award arbitrary bonus points (e.g. round complete bonus). */
	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void AddBonusPoints(int32 Points);

	/** Award points for filling a home slot. */
	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void AddHomeSlotScore();

	/** Reset the consecutive-hop multiplier to 1.0 (called on retreat, death, or reaching home). */
	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void ResetMultiplier();

	/** Lose one life. Also resets the multiplier. */
	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void LoseLife();

	/** Reset score and lives for a new game. High score persists. */
	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void StartNewGame();

	/** True when the player has zero lives remaining. */
	UFUNCTION(BlueprintPure, Category = "Scoring")
	bool IsGameOver() const;

private:
	/** Track the last extra-life threshold that was crossed to avoid double-awarding. */
	int32 LastExtraLifeThreshold = 0;

	/** Internal: update high score and broadcast score change. */
	void NotifyScoreChanged();

	/** Internal: check if score crossed an extra-life threshold. */
	void CheckExtraLife();
};
