// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/LaneTypes.h"
#include "HazardBase.generated.h"

class UBoxComponent;
class UStaticMesh;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ESubmergePhase : uint8
{
	Surface		UMETA(DisplayName = "Surface"),
	Warning		UMETA(DisplayName = "Warning"),
	Submerged	UMETA(DisplayName = "Submerged")
};

UCLASS()
class UNREALFROG_API AHazardBase : public AActor
{
	GENERATED_BODY()

public:
	AHazardBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// -- Tunable parameters -----------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	float Speed = 0.0f;

	/** Original speed from lane config. Used by ApplyWaveDifficulty to avoid compounding. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hazard")
	float BaseSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	int32 HazardWidthCells = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	bool bMovesRight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	EHazardType HazardType = EHazardType::Car;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	bool bIsRideable = false;

	// -- Turtle submerge --------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard|Turtle")
	float SurfaceDuration = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard|Turtle")
	float WarningDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard|Turtle")
	float SubmergeDuration = 2.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsSubmerged = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	ESubmergePhase SubmergePhase = ESubmergePhase::Surface;

	/** Current wave number — used to gate submerge (Wave 2+ only). Set by LaneManager. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard|Turtle")
	int32 CurrentWave = 1;

	// -- Grid references (set by LaneManager) -----------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridColumns = 13;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float GridCellSize = 100.0f;

	// -- Public interface -------------------------------------------------

	/** Moves the hazard along the X axis. Exposed for unit testing. */
	void TickMovement(float DeltaTime);

	/** Updates turtle submerge timer. Exposed for unit testing. */
	void TickSubmerge(float DeltaTime);

	/** Configure this hazard from a lane config. Called by LaneManager. */
	void InitFromConfig(const FLaneConfig& Config, float InGridCellSize, int32 InGridColumns);

protected:
	// -- Components -------------------------------------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

private:
	// -- Wrapping ---------------------------------------------------------

	/** Teleport hazard to the opposite side when it leaves the grid */
	void WrapPosition();

	/** Width of the hazard in world units */
	float GetWorldWidth() const;

	/** Left boundary for wrapping (includes buffer for hazard width) */
	float GetWrapMinX() const;

	/** Right boundary for wrapping (includes buffer for hazard width) */
	float GetWrapMaxX() const;

	// -- Mesh setup -------------------------------------------------------

	/** Configure mesh shape, scale, and color based on HazardType. Called in BeginPlay. */
	void SetupMeshForHazardType();

	/** Cached mesh assets loaded in constructor via ConstructorHelpers */
	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMeshAsset;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CylinderMeshAsset;

	// -- Turtle submerge state --------------------------------------------

	float SubmergeTimer = 0.0f;
};
