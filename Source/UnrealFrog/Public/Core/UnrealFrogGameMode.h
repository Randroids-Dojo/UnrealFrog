// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/LaneTypes.h"
#include "UnrealFrogGameMode.generated.h"

UENUM(BlueprintType)
enum class EGameState : uint8
{
	Title			UMETA(DisplayName = "Title"),
	Spawning		UMETA(DisplayName = "Spawning"),
	Playing			UMETA(DisplayName = "Playing"),
	Paused			UMETA(DisplayName = "Paused"),
	Dying			UMETA(DisplayName = "Dying"),
	RoundComplete	UMETA(DisplayName = "Round Complete"),
	GameOver		UMETA(DisplayName = "Game Over")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, EGameState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveComplete, int32, CompletedWave, int32, NextWave);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerUpdate, float, TimeRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHomeSlotFilled, int32, SlotIndex, int32, TotalFilled);

/** Non-dynamic delegates for lambda binding in tests and C++ listeners. */
DECLARE_MULTICAST_DELEGATE(FOnTimerExpiredNative);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWaveCompletedNative, int32 /*CompletedWave*/, int32 /*NextWave*/);

UCLASS()
class UNREALFROG_API AUnrealFrogGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AUnrealFrogGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// -- Tunable parameters -----------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	float TimePerLevel = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HomeSlots")
	TArray<int32> HomeSlotColumns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	float DifficultySpeedIncrement = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	int32 WavesPerGapReduction = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	float MaxSpeedMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HomeSlots")
	int32 TotalHomeSlots = 5;

	// -- Timed state transition durations ---------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float SpawningDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float DyingDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float RoundCompleteDuration = 2.0f;

	// -- Grid references --------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 HomeSlotRow = 14;

	// -- Runtime state ----------------------------------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EGameState CurrentState = EGameState::Title;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	int32 CurrentWave = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	int32 HomeSlotsFilledCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	TArray<bool> HomeSlots;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timer")
	float RemainingTime = 30.0f;

	/** Highest row the frog has reached this life (for scoring forward hops). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	int32 HighestRowReached = 0;

	/** Set by external systems (ScoreSubsystem) when lives reach 0.
	  * Checked by OnDyingComplete to decide GameOver vs Spawning. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	bool bPendingGameOver = false;

	// -- Delegates --------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGameStateChanged OnGameStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWaveComplete OnWaveCompletedBP;

	/** Native C++ delegate for wave completion (allows lambda binding in tests). */
	FOnWaveCompletedNative OnWaveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTimerUpdate OnTimerUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHomeSlotFilled OnHomeSlotFilled;

	/** Native C++ delegate for timer expiry (allows lambda binding in tests). */
	FOnTimerExpiredNative OnTimerExpired;

	// -- State transitions ------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "GameState")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "GameState")
	void PauseGame();

	UFUNCTION(BlueprintCallable, Category = "GameState")
	void ResumeGame();

	UFUNCTION(BlueprintCallable, Category = "GameState")
	void ReturnToTitle();

	/** Called externally when the score subsystem reports game over (0 lives). */
	UFUNCTION(BlueprintCallable, Category = "GameState")
	void HandleGameOver();

	// -- Home slots -------------------------------------------------------

	/** Attempt to fill a home slot at the given column. Returns true on success. */
	UFUNCTION(BlueprintCallable, Category = "HomeSlots")
	bool TryFillHomeSlot(int32 Column);

	/** True if Column matches one of the HomeSlotColumns. */
	UFUNCTION(BlueprintPure, Category = "HomeSlots")
	bool IsHomeSlotColumn(int32 Column) const;

	// -- Difficulty -------------------------------------------------------

	/** Speed multiplier for current wave: 1.0 + (Wave-1) * Increment. */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	float GetSpeedMultiplier() const;

	/** Gap reduction for current wave: (Wave-1) / WavesPerGapReduction. */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	int32 GetGapReduction() const;

	// -- Timer ------------------------------------------------------------

	/** Tick the level timer. Only counts down while in Playing state. */
	void TickTimer(float DeltaTime);

	// -- Orchestration (public for direct test invocation) ----------------

	/** Handle frog death: transition to Dying state. Only valid from Playing. */
	UFUNCTION(BlueprintCallable, Category = "Orchestration")
	void HandleFrogDied(EDeathType DeathType);

	/** Handle hop completion: track highest row, score forward hops, check home slots. */
	UFUNCTION(BlueprintCallable, Category = "Orchestration")
	void HandleHopCompleted(FIntPoint NewGridPosition);

	/** Called when Spawning timer expires → transition to Playing. */
	UFUNCTION(BlueprintCallable, Category = "Orchestration")
	void OnSpawningComplete();

	/** Called when Dying timer expires → GameOver or Spawning. */
	UFUNCTION(BlueprintCallable, Category = "Orchestration")
	void OnDyingComplete();

	/** Called when RoundComplete timer expires → next wave, Spawning. */
	UFUNCTION(BlueprintCallable, Category = "Orchestration")
	void OnRoundCompleteFinished();

private:
	/** Reset all home slots to unfilled. */
	void ResetHomeSlots();

	/** Called when all home slots are filled. */
	void OnWaveComplete();

	/** Called when the level timer reaches 0. */
	void OnTimeExpired();

	/** Set state and broadcast delegate. */
	void SetState(EGameState NewState);

	// -- Timer handles for timed state transitions ------------------------

	FTimerHandle SpawningTimerHandle;
	FTimerHandle DyingTimerHandle;
	FTimerHandle RoundCompleteTimerHandle;
};
