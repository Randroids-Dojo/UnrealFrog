// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/FrogPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Core/FrogCharacter.h"
#include "Core/UnrealFrogGameMode.h"

AFrogPlayerController::AFrogPlayerController()
{
	CreateInputActions();
}

void AFrogPlayerController::CreateInputActions()
{
	// -- Create input actions --------------------------------------------------

	IA_HopUp = NewObject<UInputAction>(this, TEXT("IA_HopUp"));
	IA_HopUp->ValueType = EInputActionValueType::Boolean;

	IA_HopDown = NewObject<UInputAction>(this, TEXT("IA_HopDown"));
	IA_HopDown->ValueType = EInputActionValueType::Boolean;

	IA_HopLeft = NewObject<UInputAction>(this, TEXT("IA_HopLeft"));
	IA_HopLeft->ValueType = EInputActionValueType::Boolean;

	IA_HopRight = NewObject<UInputAction>(this, TEXT("IA_HopRight"));
	IA_HopRight->ValueType = EInputActionValueType::Boolean;

	IA_Start = NewObject<UInputAction>(this, TEXT("IA_Start"));
	IA_Start->ValueType = EInputActionValueType::Boolean;

	IA_Pause = NewObject<UInputAction>(this, TEXT("IA_Pause"));
	IA_Pause->ValueType = EInputActionValueType::Boolean;

	// -- Create mapping context and bind keys ---------------------------------

	IMC_Frogger = NewObject<UInputMappingContext>(this, TEXT("IMC_Frogger"));

	// Hop Up: Up arrow + W
	IMC_Frogger->MapKey(IA_HopUp, EKeys::Up);
	IMC_Frogger->MapKey(IA_HopUp, EKeys::W);

	// Hop Down: Down arrow + S
	IMC_Frogger->MapKey(IA_HopDown, EKeys::Down);
	IMC_Frogger->MapKey(IA_HopDown, EKeys::S);

	// Hop Left: Left arrow + A
	IMC_Frogger->MapKey(IA_HopLeft, EKeys::Left);
	IMC_Frogger->MapKey(IA_HopLeft, EKeys::A);

	// Hop Right: Right arrow + D
	IMC_Frogger->MapKey(IA_HopRight, EKeys::Right);
	IMC_Frogger->MapKey(IA_HopRight, EKeys::D);

	// Start: Enter + Space
	IMC_Frogger->MapKey(IA_Start, EKeys::Enter);
	IMC_Frogger->MapKey(IA_Start, EKeys::SpaceBar);

	// Pause: Escape (stubbed -- full implementation deferred to Sprint 3)
	IMC_Frogger->MapKey(IA_Pause, EKeys::Escape);
}

void AFrogPlayerController::BeginPlay()
{
	Super::BeginPlay();
	AddMappingContext();
}

void AFrogPlayerController::AddMappingContext()
{
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
		{
			Subsystem->AddMappingContext(IMC_Frogger, 0);
		}
	}
}

void AFrogPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(IA_HopUp, ETriggerEvent::Started, this, &AFrogPlayerController::HandleHopUp);
		EIC->BindAction(IA_HopDown, ETriggerEvent::Started, this, &AFrogPlayerController::HandleHopDown);
		EIC->BindAction(IA_HopLeft, ETriggerEvent::Started, this, &AFrogPlayerController::HandleHopLeft);
		EIC->BindAction(IA_HopRight, ETriggerEvent::Started, this, &AFrogPlayerController::HandleHopRight);
		EIC->BindAction(IA_Start, ETriggerEvent::Started, this, &AFrogPlayerController::HandleStart);
		EIC->BindAction(IA_Pause, ETriggerEvent::Started, this, &AFrogPlayerController::HandlePause);
	}
}

// ---------------------------------------------------------------------------
// State-based input gating
// ---------------------------------------------------------------------------

bool AFrogPlayerController::CanAcceptHopInput() const
{
	if (UWorld* World = GetWorld())
	{
		if (AUnrealFrogGameMode* GM = Cast<AUnrealFrogGameMode>(World->GetAuthGameMode()))
		{
			return GM->CurrentState == EGameState::Playing;
		}
	}
	return false;
}

bool AFrogPlayerController::CanAcceptStartInput() const
{
	if (UWorld* World = GetWorld())
	{
		if (AUnrealFrogGameMode* GM = Cast<AUnrealFrogGameMode>(World->GetAuthGameMode()))
		{
			EGameState State = GM->CurrentState;
			return State == EGameState::Title || State == EGameState::GameOver;
		}
	}
	return false;
}

// ---------------------------------------------------------------------------
// Input handlers
// ---------------------------------------------------------------------------

void AFrogPlayerController::HandleHopUp()
{
	if (!CanAcceptHopInput()) return;

	if (AFrogCharacter* Frog = Cast<AFrogCharacter>(GetPawn()))
	{
		Frog->RequestHop(FVector(0.0, 1.0, 0.0));
	}
}

void AFrogPlayerController::HandleHopDown()
{
	if (!CanAcceptHopInput()) return;

	if (AFrogCharacter* Frog = Cast<AFrogCharacter>(GetPawn()))
	{
		Frog->RequestHop(FVector(0.0, -1.0, 0.0));
	}
}

void AFrogPlayerController::HandleHopLeft()
{
	if (!CanAcceptHopInput()) return;

	if (AFrogCharacter* Frog = Cast<AFrogCharacter>(GetPawn()))
	{
		Frog->RequestHop(FVector(-1.0, 0.0, 0.0));
	}
}

void AFrogPlayerController::HandleHopRight()
{
	if (!CanAcceptHopInput()) return;

	if (AFrogCharacter* Frog = Cast<AFrogCharacter>(GetPawn()))
	{
		Frog->RequestHop(FVector(1.0, 0.0, 0.0));
	}
}

void AFrogPlayerController::HandleStart()
{
	if (!CanAcceptStartInput()) return;

	if (UWorld* World = GetWorld())
	{
		if (AUnrealFrogGameMode* GM = Cast<AUnrealFrogGameMode>(World->GetAuthGameMode()))
		{
			if (GM->CurrentState == EGameState::GameOver)
			{
				GM->ReturnToTitle();
			}
			GM->StartGame();
		}
	}
}

void AFrogPlayerController::HandlePause()
{
	if (UWorld* World = GetWorld())
	{
		if (AUnrealFrogGameMode* GM = Cast<AUnrealFrogGameMode>(World->GetAuthGameMode()))
		{
			if (GM->CurrentState == EGameState::Playing)
			{
				GM->PauseGame();
			}
			else if (GM->CurrentState == EGameState::Paused)
			{
				GM->ResumeGame();
			}
		}
	}
}
