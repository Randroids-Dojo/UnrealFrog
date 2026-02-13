// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LaneTypes.generated.h"

UENUM(BlueprintType)
enum class EDeathType : uint8
{
	None		UMETA(DisplayName = "None"),
	Squish		UMETA(DisplayName = "Squish"),
	Splash		UMETA(DisplayName = "Splash"),
	Timeout		UMETA(DisplayName = "Timeout"),
	OffScreen	UMETA(DisplayName = "Off Screen")
};

UENUM(BlueprintType)
enum class ELaneType : uint8
{
	Safe	UMETA(DisplayName = "Safe"),
	Road	UMETA(DisplayName = "Road"),
	River	UMETA(DisplayName = "River"),
	Goal	UMETA(DisplayName = "Goal")
};

UENUM(BlueprintType)
enum class EHazardType : uint8
{
	Car				UMETA(DisplayName = "Car"),
	Truck			UMETA(DisplayName = "Truck"),
	Bus				UMETA(DisplayName = "Bus"),
	Motorcycle		UMETA(DisplayName = "Motorcycle"),
	SmallLog		UMETA(DisplayName = "Small Log"),
	LargeLog		UMETA(DisplayName = "Large Log"),
	TurtleGroup		UMETA(DisplayName = "Turtle Group")
};

USTRUCT(BlueprintType)
struct FLaneConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
	ELaneType LaneType = ELaneType::Safe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
	EHazardType HazardType = EHazardType::Car;

	/** Movement speed in Unreal Units per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
	float Speed = 0.0f;

	/** Width of each hazard in grid cells */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
	int32 HazardWidth = 1;

	/** Minimum gap between hazards in grid cells */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
	int32 MinGapCells = 1;

	/** True = moves in +X direction, False = moves in -X direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
	bool bMovesRight = true;

	/** Which row this lane occupies (0-14) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
	int32 RowIndex = 0;
};
