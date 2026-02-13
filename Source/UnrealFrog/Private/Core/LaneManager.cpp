// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/LaneManager.h"
#include "Core/HazardBase.h"
#include "Engine/World.h"

ALaneManager::ALaneManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ALaneManager::BeginPlay()
{
	Super::BeginPlay();

	if (LaneConfigs.Num() == 0)
	{
		SetupDefaultLaneConfigs();
	}

	for (const FLaneConfig& Config : LaneConfigs)
	{
		if (Config.LaneType == ELaneType::Road || Config.LaneType == ELaneType::River)
		{
			SpawnLaneHazards(Config);
		}
	}
}

ELaneType ALaneManager::GetLaneTypeAtRow(int32 Row) const
{
	for (const FLaneConfig& Config : LaneConfigs)
	{
		if (Config.RowIndex == Row)
		{
			return Config.LaneType;
		}
	}

	// Rows not in any config are safe by default
	return ELaneType::Safe;
}

bool ALaneManager::IsRowSafe(int32 Row) const
{
	ELaneType Type = GetLaneTypeAtRow(Row);
	return (Type == ELaneType::Safe || Type == ELaneType::Goal);
}

bool ALaneManager::ValidateGaps() const
{
	for (const FLaneConfig& Config : LaneConfigs)
	{
		if (Config.LaneType == ELaneType::Safe || Config.LaneType == ELaneType::Goal)
		{
			continue;
		}

		// A lane is impossible if a single hazard is wider than the entire grid
		// minus the minimum gap needed for the frog to pass (1 cell)
		if (Config.HazardWidth >= GridColumns)
		{
			return false;
		}

		// With multiple hazards evenly spaced, verify there is room for at least
		// one gap of MinGapCells between consecutive hazards.
		// Total space per hazard slot = HazardWidth + MinGapCells
		// Number of slots that fit = GridColumns / (HazardWidth + MinGapCells)
		// If zero slots fit, the config is invalid.
		int32 SlotSize = Config.HazardWidth + FMath::Max(Config.MinGapCells, 1);
		if (SlotSize > GridColumns)
		{
			return false;
		}
	}

	return true;
}

void ALaneManager::SetupDefaultLaneConfigs()
{
	LaneConfigs.Empty();

	// -- Road lanes (rows 1-5) --------------------------------------------

	// Row 1: Car, 150 UU/s, width 1, gap 3, LEFT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::Road;
		C.HazardType = EHazardType::Car;
		C.Speed = 150.0f;
		C.HazardWidth = 1;
		C.MinGapCells = 3;
		C.bMovesRight = false;
		C.RowIndex = 1;
		LaneConfigs.Add(C);
	}

	// Row 2: Truck, 100 UU/s, width 2, gap 4, RIGHT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::Road;
		C.HazardType = EHazardType::Truck;
		C.Speed = 100.0f;
		C.HazardWidth = 2;
		C.MinGapCells = 4;
		C.bMovesRight = true;
		C.RowIndex = 2;
		LaneConfigs.Add(C);
	}

	// Row 3: Car, 200 UU/s, width 1, gap 2, LEFT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::Road;
		C.HazardType = EHazardType::Car;
		C.Speed = 200.0f;
		C.HazardWidth = 1;
		C.MinGapCells = 2;
		C.bMovesRight = false;
		C.RowIndex = 3;
		LaneConfigs.Add(C);
	}

	// Row 4: Bus, 175 UU/s, width 2, gap 3, RIGHT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::Road;
		C.HazardType = EHazardType::Bus;
		C.Speed = 175.0f;
		C.HazardWidth = 2;
		C.MinGapCells = 3;
		C.bMovesRight = true;
		C.RowIndex = 4;
		LaneConfigs.Add(C);
	}

	// Row 5: Motorcycle, 250 UU/s, width 1, gap 2, LEFT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::Road;
		C.HazardType = EHazardType::Motorcycle;
		C.Speed = 250.0f;
		C.HazardWidth = 1;
		C.MinGapCells = 2;
		C.bMovesRight = false;
		C.RowIndex = 5;
		LaneConfigs.Add(C);
	}

	// -- River lanes (rows 7-12) ------------------------------------------

	// Row 7: SmallLog, 100 UU/s, width 2, gap 3, RIGHT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::River;
		C.HazardType = EHazardType::SmallLog;
		C.Speed = 100.0f;
		C.HazardWidth = 2;
		C.MinGapCells = 3;
		C.bMovesRight = true;
		C.RowIndex = 7;
		LaneConfigs.Add(C);
	}

	// Row 8: TurtleGroup, 80 UU/s, width 3, gap 3, LEFT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::River;
		C.HazardType = EHazardType::TurtleGroup;
		C.Speed = 80.0f;
		C.HazardWidth = 3;
		C.MinGapCells = 3;
		C.bMovesRight = false;
		C.RowIndex = 8;
		LaneConfigs.Add(C);
	}

	// Row 9: LargeLog, 120 UU/s, width 4, gap 3, RIGHT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::River;
		C.HazardType = EHazardType::LargeLog;
		C.Speed = 120.0f;
		C.HazardWidth = 4;
		C.MinGapCells = 3;
		C.bMovesRight = true;
		C.RowIndex = 9;
		LaneConfigs.Add(C);
	}

	// Row 10: SmallLog, 100 UU/s, width 2, gap 2, LEFT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::River;
		C.HazardType = EHazardType::SmallLog;
		C.Speed = 100.0f;
		C.HazardWidth = 2;
		C.MinGapCells = 2;
		C.bMovesRight = false;
		C.RowIndex = 10;
		LaneConfigs.Add(C);
	}

	// Row 11: TurtleGroup, 80 UU/s, width 3, gap 4, RIGHT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::River;
		C.HazardType = EHazardType::TurtleGroup;
		C.Speed = 80.0f;
		C.HazardWidth = 3;
		C.MinGapCells = 4;
		C.bMovesRight = true;
		C.RowIndex = 11;
		LaneConfigs.Add(C);
	}

	// Row 12: LargeLog, 150 UU/s, width 4, gap 2, LEFT
	{
		FLaneConfig C;
		C.LaneType = ELaneType::River;
		C.HazardType = EHazardType::LargeLog;
		C.Speed = 150.0f;
		C.HazardWidth = 4;
		C.MinGapCells = 2;
		C.bMovesRight = false;
		C.RowIndex = 12;
		LaneConfigs.Add(C);
	}
}

void ALaneManager::SpawnLaneHazards(const FLaneConfig& Config)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<float> XPositions;
	CalculateSpawnPositions(Config, XPositions);

	TArray<AHazardBase*>& LaneHazards = HazardPool.FindOrAdd(Config.RowIndex);

	float YPos = static_cast<float>(Config.RowIndex) * GridCellSize;

	for (float XPos : XPositions)
	{
		FVector SpawnLocation(XPos, YPos, 0.0f);
		FRotator SpawnRotation = FRotator::ZeroRotator;

		AHazardBase* Hazard = World->SpawnActor<AHazardBase>(
			AHazardBase::StaticClass(),
			SpawnLocation,
			SpawnRotation
		);

		if (Hazard)
		{
			Hazard->InitFromConfig(Config, GridCellSize, GridColumns);
			LaneHazards.Add(Hazard);
		}
	}
}

void ALaneManager::CalculateSpawnPositions(const FLaneConfig& Config, TArray<float>& OutXPositions) const
{
	float GridWorldWidth = static_cast<float>(GridColumns) * GridCellSize;
	float HazardWorldWidth = static_cast<float>(Config.HazardWidth) * GridCellSize;
	float GapWorldWidth = static_cast<float>(FMath::Max(Config.MinGapCells, 1)) * GridCellSize;

	// Evenly space hazards across the lane width, accounting for wrapping
	float SlotWidth = HazardWorldWidth + GapWorldWidth;
	int32 NumHazards = FMath::Max(1, FMath::FloorToInt(GridWorldWidth / SlotWidth));
	NumHazards = FMath::Min(NumHazards, HazardsPerLane);

	float Spacing = GridWorldWidth / static_cast<float>(NumHazards);

	for (int32 i = 0; i < NumHazards; ++i)
	{
		float XPos = static_cast<float>(i) * Spacing;
		OutXPositions.Add(XPos);
	}
}
