// Copyright UnrealFrog Team. All Rights Reserved.

#include "FroggerFunctionalTest.h"
#include "Core/FrogCharacter.h"
#include "Core/UnrealFrogGameMode.h"
#include "Kismet/GameplayStatics.h"

AFroggerFunctionalTest::AFroggerFunctionalTest()
{
}

void AFroggerFunctionalTest::PrepareTest()
{
	Super::PrepareTest();

	// Cache references to GameMode and Frog
	UWorld* World = GetWorld();
	if (World)
	{
		CachedGameMode = Cast<AUnrealFrogGameMode>(World->GetAuthGameMode());

		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
		CachedFrog = Cast<AFrogCharacter>(PlayerPawn);
	}
}

void AFroggerFunctionalTest::SimulateHop(FVector Direction)
{
	AFrogCharacter* Frog = GetFrog();
	if (!Frog)
	{
		LogMessage(TEXT("SimulateHop: No frog character found!"));
		FinishTest(EFunctionalTestResult::Failed, TEXT("No frog to hop"));
		return;
	}

	Frog->RequestHop(Direction);
}

void AFroggerFunctionalTest::TickForDuration(float Duration)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Tick in small increments to allow physics and overlaps to process
	const float StepSize = 1.0f / 60.0f;
	float Elapsed = 0.0f;
	while (Elapsed < Duration)
	{
		float DT = FMath::Min(StepSize, Duration - Elapsed);
		World->Tick(LEVELTICK_All, DT);
		Elapsed += DT;
	}
}

void AFroggerFunctionalTest::AssertFrogAt(FIntPoint ExpectedGridPos)
{
	AFrogCharacter* Frog = GetFrog();
	if (!Frog)
	{
		LogMessage(TEXT("AssertFrogAt: No frog character found!"));
		FinishTest(EFunctionalTestResult::Failed, TEXT("No frog found"));
		return;
	}

	if (Frog->GridPosition != ExpectedGridPos)
	{
		FString Msg = FString::Printf(
			TEXT("Expected frog at (%d,%d) but was at (%d,%d)"),
			ExpectedGridPos.X, ExpectedGridPos.Y,
			Frog->GridPosition.X, Frog->GridPosition.Y);
		LogMessage(Msg);
		FinishTest(EFunctionalTestResult::Failed, Msg);
	}
}

void AFroggerFunctionalTest::AssertGameState(EGameState ExpectedState)
{
	AUnrealFrogGameMode* GM = GetGameMode();
	if (!GM)
	{
		LogMessage(TEXT("AssertGameState: No game mode found!"));
		FinishTest(EFunctionalTestResult::Failed, TEXT("No game mode found"));
		return;
	}

	if (GM->CurrentState != ExpectedState)
	{
		FString Msg = FString::Printf(
			TEXT("Expected state %d but was %d"),
			static_cast<int32>(ExpectedState),
			static_cast<int32>(GM->CurrentState));
		LogMessage(Msg);
		FinishTest(EFunctionalTestResult::Failed, Msg);
	}
}

AFrogCharacter* AFroggerFunctionalTest::GetFrog() const
{
	return CachedFrog.Get();
}

AUnrealFrogGameMode* AFroggerFunctionalTest::GetGameMode() const
{
	return CachedGameMode.Get();
}

void AFroggerFunctionalTest::StartGameAndWaitForPlaying()
{
	AUnrealFrogGameMode* GM = GetGameMode();
	if (!GM)
	{
		LogMessage(TEXT("StartGameAndWaitForPlaying: No game mode!"));
		FinishTest(EFunctionalTestResult::Failed, TEXT("No game mode"));
		return;
	}

	GM->StartGame();
	GM->OnSpawningComplete();
}
