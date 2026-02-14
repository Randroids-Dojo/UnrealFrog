// Copyright UnrealFrog Team. All Rights Reserved.

#include "FroggerFunctionalTests.h"
#include "Core/FrogCharacter.h"
#include "Core/UnrealFrogGameMode.h"

void AFT_RiverDeath::StartTest()
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

	// Do NOT spawn any logs — the river row is empty

	// Hop forward onto the empty river
	SimulateHop(FVector(0.0, 1.0, 0.0));
	TickForDuration(Frog->HopDuration + 0.05f);

	if (!Frog->bIsDead)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Frog should die on empty river"));
		return;
	}

	if (Frog->LastDeathType != EDeathType::Splash)
	{
		FString Msg = FString::Printf(TEXT("Expected Splash death, got %d"),
			static_cast<int32>(Frog->LastDeathType));
		FinishTest(EFunctionalTestResult::Failed, Msg);
		return;
	}

	FinishTest(EFunctionalTestResult::Succeeded, TEXT("Frog died by splash on empty river"));
}
