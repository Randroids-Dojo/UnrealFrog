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

private:
	// -- Drawing helpers --------------------------------------------------

	void DrawScorePanel();
	void DrawTimerBar();
	void DrawOverlay();
};
