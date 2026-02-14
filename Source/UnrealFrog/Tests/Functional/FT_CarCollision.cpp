// Copyright UnrealFrog Team. All Rights Reserved.

#include "FroggerFunctionalTests.h"
#include "Core/FrogCharacter.h"
#include "Core/HazardBase.h"
#include "Core/ScoreSubsystem.h"
#include "Core/UnrealFrogGameMode.h"

void AFT_CarCollision::StartTest()
{
	StartGameAndWaitForPlaying();

	AFrogCharacter* Frog = GetFrog();
	AUnrealFrogGameMode* GM = GetGameMode();

	if (!Frog || !GM)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Missing Frog or GameMode"));
		return;
	}

	// Spawn a non-rideable car hazard at row 1
	UWorld* World = GetWorld();
	AHazardBase* Car = World->SpawnActor<AHazardBase>(AHazardBase::StaticClass());
	if (!Car)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Failed to spawn car"));
		return;
	}

	Car->bIsRideable = false;
	Car->HazardType = EHazardType::Car;
	Car->Speed = 0.0f;
	Car->SetActorLocation(Frog->GridToWorld(FIntPoint(6, 1)));

	int32 InitialLives = 3;
	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (UScoreSubsystem* Scoring = GI->GetSubsystem<UScoreSubsystem>())
		{
			InitialLives = Scoring->Lives;
		}
	}

	// Hop into the car
	SimulateHop(FVector(0.0, 1.0, 0.0));
	TickForDuration(Frog->HopDuration + 0.05f);

	if (!Frog->bIsDead)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Frog should be dead after car collision"));
		return;
	}

	if (Frog->LastDeathType != EDeathType::Squish)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Death type should be Squish"));
		return;
	}

	FinishTest(EFunctionalTestResult::Succeeded, TEXT("Car collision killed frog"));
}
