// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Core/LaneTypes.h"
#include "FrogCharacter.generated.h"

class UAudioComponent;
class UCapsuleComponent;
class UStaticMeshComponent;
class AHazardBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHopCompleted, FIntPoint, NewGridPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFrogDied, EDeathType, DeathType);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFrogDiedNative, EDeathType);
DECLARE_MULTICAST_DELEGATE(FOnHopStartedNative);

UCLASS()
class UNREALFROG_API AFrogCharacter : public APawn
{
	GENERATED_BODY()

public:
	AFrogCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// -- Tunable parameters (EditAnywhere for designer iteration) ----------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float HopDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float HopArcHeight = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float GridCellSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float InputBufferWindow = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BackwardHopMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridColumns = 13;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridRows = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	float RespawnDelay = 1.0f;

	/** River rows: rows 7 through 12 inclusive (set by level design) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 RiverRowMin = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 RiverRowMax = 12;

	// -- Runtime state ----------------------------------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	FIntPoint GridPosition = FIntPoint(6, 0);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsHopping = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsGameOver = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EDeathType LastDeathType = EDeathType::None;

	/** The river platform the frog is currently riding (nullptr if none) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	TWeakObjectPtr<AHazardBase> CurrentPlatform;

	// -- Delegates --------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHopCompleted OnHopCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFrogDied OnFrogDied;

	/** Native C++ delegate for lambda binding (tests, C++ listeners). Not exposed to Blueprint. */
	FOnFrogDiedNative OnFrogDiedNative;

	/** Native C++ delegate that fires when a hop starts (for audio). */
	FOnHopStartedNative OnHopStartedNative;

	// -- Public interface -------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void RequestHop(FVector Direction);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	FVector GridToWorld(FIntPoint GridPos) const;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	FIntPoint WorldToGrid(FVector WorldPos) const;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool IsValidGridPosition(FIntPoint GridPos) const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	float GetHopDurationForDirection(FVector Direction) const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool HasBufferedInput() const;

	// -- Death and respawn ------------------------------------------------

	/** Kill the frog with the given death type. Broadcasts OnFrogDied. */
	UFUNCTION(BlueprintCallable, Category = "Death")
	void Die(EDeathType DeathType);

	/** Reset frog to start position and clear dead state. */
	UFUNCTION(BlueprintCallable, Category = "Death")
	void Respawn();

	/** True if the frog's current grid row is a river row (RiverRowMin..RiverRowMax). */
	UFUNCTION(BlueprintPure, Category = "Collision")
	bool IsOnRiverRow() const;

	/** True if the frog is on a river row without a valid rideable platform. */
	UFUNCTION(BlueprintPure, Category = "Collision")
	bool CheckRiverDeath() const;

	/** True if the frog's world X is outside the grid boundaries. */
	UFUNCTION(BlueprintPure, Category = "Collision")
	bool IsOffScreen() const;

	/** True if the frog should respawn (not game over). */
	UFUNCTION(BlueprintPure, Category = "Death")
	bool ShouldRespawn() const;

	/** Apply the current platform's velocity to the frog. Call each frame while riding. */
	void UpdateRiding(float DeltaTime);

	// -- Collision handlers (public for direct test invocation) ------------

	/** Process overlap with a hazard: road hazards kill, river platforms mount. */
	void HandleHazardOverlap(AHazardBase* Hazard);

	/** Process end of overlap with a platform: clear CurrentPlatform if it matches. */
	void HandlePlatformEndOverlap(AHazardBase* Hazard);

protected:
	// -- Components -------------------------------------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCapsuleComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> AudioComponent;

private:
	// -- Hop interpolation state ------------------------------------------

	FVector HopStartLocation = FVector::ZeroVector;
	FVector HopEndLocation = FVector::ZeroVector;
	float HopElapsed = 0.0f;
	float CurrentHopDuration = 0.0f;

	// -- Input buffer state -----------------------------------------------

	bool bHasBufferedInput = false;
	FVector BufferedDirection = FVector::ZeroVector;
	float BufferedInputTime = 0.0f;

	// -- Internal helpers -------------------------------------------------

	void StartHop(FVector Direction);
	void FinishHop();
	FIntPoint DirectionToGridDelta(FVector Direction) const;

	// -- Overlap event callbacks ------------------------------------------

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Synchronous overlap query to find a rideable platform at the frog's current position. */
	void FindPlatformAtCurrentPosition();

	// -- Respawn timer ----------------------------------------------------

	FTimerHandle RespawnTimerHandle;
};
