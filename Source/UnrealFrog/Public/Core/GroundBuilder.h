// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GroundBuilder.generated.h"

class UMaterialInstanceDynamic;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * Describes a single ground row: its index and color.
 */
USTRUCT(BlueprintType)
struct FGroundRowInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
	int32 Row = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
	FLinearColor Color = FLinearColor::White;
};

/**
 * Spawns the visible ground plane geometry for the Frogger grid.
 *
 * 15 flat box meshes (one per row) using /Engine/BasicShapes/Cube,
 * scaled to 1300x100x10 UU, colored per zone type. Also spawns
 * 5 small home-slot indicators on the goal row.
 *
 * Place one of these in the level. All geometry spawns in BeginPlay.
 */
UCLASS()
class UNREALFROG_API AGroundBuilder : public AActor
{
	GENERATED_BODY()

public:
	AGroundBuilder();

	virtual void BeginPlay() override;

	// -- Configuration ----------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridColumns = 13;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridRows = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float GridCellSize = 100.0f;

	/** Row color definitions. Populated with defaults in the constructor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
	TArray<FGroundRowInfo> RowDefinitions;

	/** Home slot column indices on the goal row (row 14). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
	TArray<int32> HomeSlotColumns;

	/** Home slot indicator color -- brighter green to stand out from the dark green goal row. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
	FLinearColor HomeSlotColor = FLinearColor(0.3f, 0.9f, 0.3f);

	/** Row on which home slot indicators are placed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
	int32 HomeSlotRow = 14;

	/** Cached reference to /Engine/BasicShapes/Cube -- loaded in constructor. */
	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

private:
	/** Spawn a single row plane at the given row index with the given color. */
	void SpawnRowPlane(int32 Row, const FLinearColor& Color);

	/** Spawn a small home-slot indicator plane at the given column on HomeSlotRow. */
	void SpawnHomeSlotIndicator(int32 Column);

	/** Populate RowDefinitions with the default Frogger color layout. */
	void SetupDefaultRowDefinitions();
};
