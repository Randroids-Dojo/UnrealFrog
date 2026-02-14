// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "Core/UnrealFrogGameMode.h"
#include "FroggerFunctionalTest.generated.h"

class AFrogCharacter;

/**
 * Base class for Frogger functional tests that run inside the engine with
 * full actor lifecycle, input simulation, and physics.
 *
 * Provides common setup: spawns GameMode dependencies, locates the frog,
 * and offers helpers for simulating hops and waiting for state changes.
 *
 * Subclass this and override PrepareTest/StartTest to write test scenarios.
 */
UCLASS()
class AFroggerFunctionalTest : public AFunctionalTest
{
	GENERATED_BODY()

public:
	AFroggerFunctionalTest();

	virtual void PrepareTest() override;

	// -- Test helpers ---------------------------------------------------------

	/** Simulate a hop in the given direction. Calls RequestHop on the frog. */
	UFUNCTION(BlueprintCallable, Category = "FroggerTest")
	void SimulateHop(FVector Direction);

	/** Tick the world forward by the given duration to complete a hop. */
	UFUNCTION(BlueprintCallable, Category = "FroggerTest")
	void TickForDuration(float Duration);

	/** Assert the frog is at the expected grid position. Logs error and fails if not. */
	UFUNCTION(BlueprintCallable, Category = "FroggerTest")
	void AssertFrogAt(FIntPoint ExpectedGridPos);

	/** Assert the current game state matches. Logs error and fails if not. */
	UFUNCTION(BlueprintCallable, Category = "FroggerTest")
	void AssertGameState(EGameState ExpectedState);

	/** Get the frog character. May be null if not yet spawned. */
	UFUNCTION(BlueprintPure, Category = "FroggerTest")
	AFrogCharacter* GetFrog() const;

	/** Get the game mode. May be null. */
	UFUNCTION(BlueprintPure, Category = "FroggerTest")
	AUnrealFrogGameMode* GetGameMode() const;

	/** Start the game (Title → Spawning → Playing) and wait for Playing state. */
	UFUNCTION(BlueprintCallable, Category = "FroggerTest")
	void StartGameAndWaitForPlaying();

protected:
	UPROPERTY()
	TWeakObjectPtr<AFrogCharacter> CachedFrog;

	UPROPERTY()
	TWeakObjectPtr<AUnrealFrogGameMode> CachedGameMode;
};
