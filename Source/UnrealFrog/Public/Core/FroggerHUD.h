// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Core/UnrealFrogGameMode.h"
#include "FroggerHUD.generated.h"

/**
 * Canvas-based HUD for the Frogger game.
 * Draws score, high score, lives, wave, timer bar, and state overlays.
 * All rendering is done in DrawHUD using the Canvas API (no Blueprint required).
 */
UCLASS()
class UNREALFROG_API AFroggerHUD : public AHUD
{
	GENERATED_BODY()

public:
	AFroggerHUD();

	virtual void DrawHUD() override;

	// -- Display state (public for test access and delegate binding) -------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD")
	int32 DisplayScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD")
	int32 DisplayHighScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD")
	int32 DisplayLives = 3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD")
	int32 DisplayWave = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD")
	float TimerPercent = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD")
	EGameState DisplayState = EGameState::Title;

	// -- Update methods (called by delegates or directly in tests) ---------

	void UpdateScore(int32 NewScore);
	void UpdateHighScore(int32 NewHighScore);
	void UpdateLives(int32 NewLives);
	void UpdateWave(int32 Wave);
	void UpdateTimer(float TimeRemaining, float MaxTime);
	void UpdateGameState(EGameState NewState);

	/** Returns the overlay text for the current game state. Empty when no overlay should show. */
	FString GetOverlayText() const;

	// -- Score pop animation state (public for test access) ---------------

	struct FScorePop
	{
		FString Text;
		FVector2D Position;
		float SpawnTime = 0.0f;
		float Duration = 1.5f;
		FColor Color = FColor::Yellow;
	};

	TArray<FScorePop> ActiveScorePops;
	int32 PreviousScore = 0;

	/** Create a score pop at the frog's projected screen position.
	 *  Falls back to screen center if no player controller available. */
	void CreateScorePop(int32 Delta);

	// -- Timer pulse state (public for test access) -----------------------

	float TimerPulseAlpha = 0.0f;
	bool bTimerPulsing = false;

	// -- Wave announcement state (public for test access) -----------------

	FString WaveAnnounceText;
	float WaveAnnounceStartTime = 0.0f;
	bool bShowingWaveAnnounce = false;
	int32 PreviousWave = 1;

	// -- Wave fanfare ceremony state (public for test access) -------------

	/** Current scale of the wave fanfare text (2.0 = 200%, animates to 1.0). */
	float WaveFanfareScale = 1.0f;

	/** Timer tracking elapsed fanfare time. */
	float WaveFanfareTimer = 0.0f;

	/** Duration of the fanfare scale animation. Must fit within RoundCompleteDuration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Fanfare")
	float WaveFanfareDuration = 1.5f;

	/** Screen flash alpha for wave fanfare (white overlay that fades). */
	float WaveFanfareFlashAlpha = 0.0f;

	// -- Death flash state (public for test access) -------------------------

	float DeathFlashAlpha = 0.0f;

	/** Tick the death flash decay. Call each frame with DeltaTime. */
	void TickDeathFlash(float DeltaTime);

private:
	// -- Drawing helpers --------------------------------------------------

	void DrawScorePanel();
	void DrawTimerBar();
	void DrawOverlay();
	void DrawScorePops();
	void DrawWaveAnnouncement();
	void DrawTitleScreen();
	void DrawDeathFlash();
};
