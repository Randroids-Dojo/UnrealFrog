// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/LaneTypes.h"
#include "FroggerVFXManager.generated.h"

/** Describes a single active VFX instance with its animation state. */
USTRUCT()
struct FActiveVFX
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AActor> Actor;

	FVector SpawnLocation = FVector::ZeroVector;
	float SpawnTime = 0.0f;
	float Duration = 0.0f;
	float StartScale = 0.5f;
	float EndScale = 3.0f;
	FVector RiseVelocity = FVector::ZeroVector;
};

/**
 * Geometry-based VFX manager. Spawns temporary mesh actors for visual
 * feedback on game events. No Niagara or .uasset particle systems needed.
 *
 * Effects use scale-and-destroy animation: spawn at StartScale,
 * lerp to EndScale over Duration, then destroy.
 */
UCLASS()
class UNREALFROG_API UFroggerVFXManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** When true, all Spawn methods are no-ops. Use for CI and headless testing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
	bool bDisabled = false;

	/** Maximum active VFX actors. Oldest destroyed when cap reached. */
	static constexpr int32 MaxActiveVFX = 10;

	// -- VFX spawn interface -----------------------------------------------

	/** Expanding sphere, color by death type. 0.5s duration. */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SpawnDeathPuff(FVector Location, EDeathType DeathType);

	/** 3 small cubes at hop origin, quick pop. 0.2s duration. */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SpawnHopDust(FVector Location);

	/** 4 small spheres rising upward, golden yellow. 1.0s duration. */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SpawnHomeSlotSparkle(FVector Location);

	/** Sparkles at all 5 home slot positions. 2.0s duration. */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SpawnRoundCompleteCelebration();

	/** Tick-driven animation: scale, move, and destroy completed effects. */
	void TickVFX(float CurrentTime);

	// -- Delegate handler adapters -----------------------------------------

	UFUNCTION()
	void HandleHomeSlotFilled(int32 SlotIndex, int32 TotalFilled);

	// -- State access (for tests) ------------------------------------------

	UPROPERTY()
	TArray<FActiveVFX> ActiveEffects;

private:
	/** Spawn a mesh actor at a location with a given shape, color, and scale. */
	AActor* SpawnVFXActor(UWorld* World, FVector Location, const TCHAR* MeshPath,
		FLinearColor Color, float Scale);

	/** Enforce the max active VFX cap by destroying the oldest. */
	void EnforceActiveCap();
};
