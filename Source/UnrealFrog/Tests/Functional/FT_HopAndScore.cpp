// Copyright UnrealFrog Team. All Rights Reserved.

#include "FroggerFunctionalTests.h"
#include "Core/FrogCharacter.h"
#include "Core/ScoreSubsystem.h"
#include "Core/UnrealFrogGameMode.h"

void AFT_HopAndScore::StartTest()
{
	StartGameAndWaitForPlaying();

	AFrogCharacter* Frog = GetFrog();
	AUnrealFrogGameMode* GM = GetGameMode();

	if (!Frog || !GM)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Missing Frog or GameMode"));
		return;
	}

	// Hop forward 3 times
	for (int32 i = 0; i < 3; i++)
	{
		SimulateHop(FVector(0.0, 1.0, 0.0));
		TickForDuration(Frog->HopDuration + 0.05f);
	}

	// Verify frog position
	AssertFrogAt(FIntPoint(6, 3));

	// Verify score via subsystem
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UScoreSubsystem* Scoring = GI->GetSubsystem<UScoreSubsystem>())
		{
			if (Scoring->Score != 60)
			{
				FString Msg = FString::Printf(TEXT("Expected score 60, got %d"), Scoring->Score);
				FinishTest(EFunctionalTestResult::Failed, Msg);
				return;
			}
		}
	}

	FinishTest(EFunctionalTestResult::Succeeded, TEXT("3 forward hops scored correctly"));
}
