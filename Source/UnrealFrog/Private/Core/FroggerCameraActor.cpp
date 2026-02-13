// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/FroggerCameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"

AFroggerCameraActor::AFroggerCameraActor()
{
	PrimaryActorTick.bCanEverTick = false;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	RootComponent = CameraComponent;

	CameraComponent->SetRelativeLocation(CameraPosition);
	CameraComponent->SetRelativeRotation(FRotator(CameraPitch, 0.0f, 0.0f));
	CameraComponent->FieldOfView = CameraFOV;
}

void AFroggerCameraActor::BeginPlay()
{
	Super::BeginPlay();

	// Apply tunable values in case they were changed in the editor
	CameraComponent->SetRelativeLocation(CameraPosition);
	CameraComponent->SetRelativeRotation(FRotator(CameraPitch, 0.0f, 0.0f));
	CameraComponent->FieldOfView = CameraFOV;

	// Auto-activate: set this actor as the view target for the first local player
	if (UWorld* World = GetWorld())
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC)
		{
			PC->SetViewTarget(this);
		}
	}
}
