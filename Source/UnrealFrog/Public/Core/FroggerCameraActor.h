// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FroggerCameraActor.generated.h"

class UCameraComponent;

/**
 * Fixed top-down camera for the Frogger grid.
 *
 * Positioned above the grid center at a -72 degree pitch to give
 * slight depth perception while showing all 13x15 cells.
 * Auto-activates as the view target in BeginPlay.
 */
UCLASS()
class UNREALFROG_API AFroggerCameraActor : public AActor
{
	GENERATED_BODY()

public:
	AFroggerCameraActor();

	virtual void BeginPlay() override;

	// -- Components -------------------------------------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComponent;

	// -- Tunable parameters -----------------------------------------------

	/** World position of the camera (centered above the 13x15 grid). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector CameraPosition = FVector(600.0, 700.0, 2200.0);

	/** Camera rotation: pitch=-90 for top-down, yaw=0. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FRotator CameraRotation = FRotator(-90.0f, 0.0f, 0.0f);

	/** Horizontal field of view in degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraFOV = 50.0f;
};
