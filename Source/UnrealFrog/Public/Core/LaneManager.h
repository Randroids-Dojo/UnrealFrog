// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/LaneTypes.h"
#include "LaneManager.generated.h"

class AHazardBase;

UCLASS()
class UNREALFROG_API ALaneManager : public AActor
{
	GENERATED_BODY()

public:
	ALaneManager();

	virtual void BeginPlay() override;

	// -- Configuration ----------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridColumns = 13;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float GridCellSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lanes")
	TArray<FLaneConfig> LaneConfigs;

	/** Number of hazards to spawn per lane (controls density) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lanes", meta = (ClampMin = "1"))
	int32 HazardsPerLane = 3;

	// -- Public interface -------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Lanes")
	ELaneType GetLaneTypeAtRow(int32 Row) const;

	UFUNCTION(BlueprintCallable, Category = "Lanes")
	bool IsRowSafe(int32 Row) const;

	UFUNCTION(BlueprintCallable, Category = "Lanes")
	bool ValidateGaps() const;

	/** Populate LaneConfigs with the default Frogger level layout */
	UFUNCTION(BlueprintCallable, Category = "Lanes")
	void SetupDefaultLaneConfigs();

	/** Apply wave difficulty scaling to all spawned hazards.
	 *  Sets Speed = BaseSpeed * SpeedMultiplier on each hazard (no compounding).
	 *  Updates turtle CurrentWave for submerge gating. */
	UFUNCTION(BlueprintCallable, Category = "Lanes")
	void ApplyWaveDifficulty(float SpeedMultiplier, int32 WaveNumber);

	/** Add a hazard to the pool for a given row. Exposed for unit testing. */
	void AddHazardToPool(int32 RowIndex, AHazardBase* Hazard);

private:
	// -- Object pool (one array of hazards per lane index) ----------------

	TMap<int32, TArray<AHazardBase*>> HazardPool;

	/** Spawn all hazards for a single lane config */
	void SpawnLaneHazards(const FLaneConfig& Config);

	/** Calculate evenly spaced starting positions for hazards in a lane */
	void CalculateSpawnPositions(const FLaneConfig& Config, TArray<float>& OutXPositions) const;
};
