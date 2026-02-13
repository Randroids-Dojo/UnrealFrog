// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FrogPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 * Player controller for the frog character.
 * Creates Enhanced Input actions and mapping context entirely in C++ (no .uasset files).
 * Routes arrow keys + WASD to AFrogCharacter::RequestHop(Direction).
 * Enter/Space triggers Start (Title->Playing, GameOver->Title).
 * Input is blocked during Dying, Spawning, RoundComplete states.
 */
UCLASS()
class UNREALFROG_API AFrogPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFrogPlayerController();

	virtual void BeginPlay() override;

	// -- Input actions (public for test access) ----------------------------

	UPROPERTY()
	TObjectPtr<UInputAction> IA_HopUp;

	UPROPERTY()
	TObjectPtr<UInputAction> IA_HopDown;

	UPROPERTY()
	TObjectPtr<UInputAction> IA_HopLeft;

	UPROPERTY()
	TObjectPtr<UInputAction> IA_HopRight;

	UPROPERTY()
	TObjectPtr<UInputAction> IA_Start;

	UPROPERTY()
	TObjectPtr<UInputAction> IA_Pause;

	// -- Mapping context (public for test access) --------------------------

	UPROPERTY()
	TObjectPtr<UInputMappingContext> IMC_Frogger;

protected:
	virtual void SetupInputComponent() override;

private:
	/** Create all input actions and the mapping context with key bindings. */
	void CreateInputActions();

	/** Register the mapping context with the Enhanced Input subsystem. */
	void AddMappingContext();

	// -- Input handlers (must be UFUNCTION for AddDynamic-style binding) ---

	UFUNCTION()
	void HandleHopUp();

	UFUNCTION()
	void HandleHopDown();

	UFUNCTION()
	void HandleHopLeft();

	UFUNCTION()
	void HandleHopRight();

	UFUNCTION()
	void HandleStart();

	UFUNCTION()
	void HandlePause();

	/** Returns true if the game is in a state that accepts hop input. */
	bool CanAcceptHopInput() const;

	/** Returns true if the game is in a state that accepts start input. */
	bool CanAcceptStartInput() const;
};
