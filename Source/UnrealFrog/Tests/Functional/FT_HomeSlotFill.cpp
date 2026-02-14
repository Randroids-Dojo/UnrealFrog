// Copyright UnrealFrog Team. All Rights Reserved.

#include "FroggerFunctionalTests.h"
#include "Core/FrogCharacter.h"
#include "Core/ScoreSubsystem.h"
#include "Core/UnrealFrogGameMode.h"

void AFT_HomeSlotFill::StartTest()
{
	StartGameAndWaitForPlaying();

	AFrogCharacter* Frog = GetFrog();
	AUnrealFrogGameMode* GM = GetGameMode();

	if (!Frog || !GM)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Missing Frog or GameMode"));
		return;
	}

	int32 ScoreBefore = 0;
	UWorld* World = GetWorld();
	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (UScoreSubsystem* Scoring = GI->GetSubsystem<UScoreSubsystem>())
		{
			ScoreBefore = Scoring->Score;
		}
	}

	// Teleport frog to one row below the home slot row
	Frog->GridPosition = FIntPoint(1, GM->HomeSlotRow - 1);
	Frog->SetActorLocation(Frog->GridToWorld(Frog->GridPosition));

	// Hop forward to the home slot row
	SimulateHop(FVector(0.0, 1.0, 0.0));
	TickForDuration(Frog->HopDuration + 0.05f);

	if (GM->HomeSlotsFilledCount != 1)
	{
		FString Msg = FString::Printf(TEXT("Expected 1 slot filled, got %d"), GM->HomeSlotsFilledCount);
		FinishTest(EFunctionalTestResult::Failed, Msg);
		return;
	}

	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (UScoreSubsystem* Scoring = GI->GetSubsystem<UScoreSubsystem>())
		{
			int32 ScoreGain = Scoring->Score - ScoreBefore;
			if (ScoreGain < 200)
			{
				FString Msg = FString::Printf(TEXT("Expected at least 200 points for home slot, got %d gain"), ScoreGain);
				FinishTest(EFunctionalTestResult::Failed, Msg);
				return;
			}
		}
	}

	FinishTest(EFunctionalTestResult::Succeeded, TEXT("Home slot filled and scored correctly"));
}
