// Copyright UnrealFrog Team. All Rights Reserved.

#include "FroggerFunctionalTests.h"
#include "Core/FrogCharacter.h"
#include "Core/HazardBase.h"
#include "Core/UnrealFrogGameMode.h"

void AFT_RiverLanding::StartTest()
{
	StartGameAndWaitForPlaying();

	AFrogCharacter* Frog = GetFrog();
	AUnrealFrogGameMode* GM = GetGameMode();

	if (!Frog || !GM)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Missing Frog or GameMode"));
		return;
	}

	// Teleport frog to just below a river row
	int32 RiverRow = Frog->RiverRowMin;
	Frog->GridPosition = FIntPoint(6, RiverRow - 1);
	Frog->SetActorLocation(Frog->GridToWorld(Frog->GridPosition));

	// Spawn a rideable log at the river row
	UWorld* World = GetWorld();
	AHazardBase* Log = World->SpawnActor<AHazardBase>(AHazardBase::StaticClass());
	if (!Log)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Failed to spawn log"));
		return;
	}

	Log->bIsRideable = true;
	Log->HazardType = EHazardType::SmallLog;
	Log->HazardWidthCells = 3;
	Log->Speed = 0.0f;
	Log->GridCellSize = Frog->GridCellSize;
	Log->SetActorLocation(Frog->GridToWorld(FIntPoint(6, RiverRow)));

	// Hop forward onto the log
	SimulateHop(FVector(0.0, 1.0, 0.0));
	TickForDuration(Frog->HopDuration + 0.05f);

	if (Frog->bIsDead)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Frog died on log — should survive"));
		return;
	}

	if (Frog->CurrentPlatform.Get() != Log)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Frog should be mounted on the log"));
		return;
	}

	FinishTest(EFunctionalTestResult::Succeeded, TEXT("Frog safely landed on river log"));
}
