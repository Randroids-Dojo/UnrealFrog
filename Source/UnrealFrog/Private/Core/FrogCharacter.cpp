// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/FrogCharacter.h"
#include "Core/HazardBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

AFrogCharacter::AFrogCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitCapsuleSize(34.0f, 44.0f);
	SetRootComponent(CollisionComponent);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);
}

void AFrogCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Snap to the initial grid position
	SetActorLocation(GridToWorld(GridPosition));
}

void AFrogCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead)
	{
		return;
	}

	if (bIsHopping)
	{
		HopElapsed += DeltaTime;
		float Alpha = FMath::Clamp(HopElapsed / CurrentHopDuration, 0.0f, 1.0f);

		// XY: linear interpolation
		FVector CurrentPos = FMath::Lerp(HopStartLocation, HopEndLocation, Alpha);

		// Z: parabolic arc -- peak at Alpha=0.5
		float ArcAlpha = 4.0f * Alpha * (1.0f - Alpha); // 0 at endpoints, 1 at midpoint
		CurrentPos.Z = HopStartLocation.Z + HopArcHeight * ArcAlpha;

		SetActorLocation(CurrentPos);

		if (Alpha >= 1.0f)
		{
			FinishHop();
		}

		return;
	}

	// When not hopping, apply platform riding on river rows
	if (IsOnRiverRow())
	{
		UpdateRiding(DeltaTime);

		// Check off-screen death after riding moves the frog
		if (IsOffScreen())
		{
			Die(EDeathType::OffScreen);
		}
	}
}

void AFrogCharacter::RequestHop(FVector Direction)
{
	if (bIsDead)
	{
		return;
	}

	if (bIsHopping)
	{
		// Buffer the input if we are mid-hop
		bHasBufferedInput = true;
		BufferedDirection = Direction;
		return;
	}

	FIntPoint Delta = DirectionToGridDelta(Direction);
	FIntPoint TargetPos = GridPosition + Delta;

	if (!IsValidGridPosition(TargetPos))
	{
		return;
	}

	StartHop(Direction);
}

FVector AFrogCharacter::GridToWorld(FIntPoint GridPos) const
{
	return FVector(
		static_cast<float>(GridPos.X) * GridCellSize,
		static_cast<float>(GridPos.Y) * GridCellSize,
		0.0f
	);
}

FIntPoint AFrogCharacter::WorldToGrid(FVector WorldPos) const
{
	return FIntPoint(
		FMath::RoundToInt(WorldPos.X / GridCellSize),
		FMath::RoundToInt(WorldPos.Y / GridCellSize)
	);
}

bool AFrogCharacter::IsValidGridPosition(FIntPoint GridPos) const
{
	return GridPos.X >= 0
		&& GridPos.X < GridColumns
		&& GridPos.Y >= 0
		&& GridPos.Y < GridRows;
}

float AFrogCharacter::GetHopDurationForDirection(FVector Direction) const
{
	// Backward = negative Y direction
	if (Direction.Y < 0.0f)
	{
		return HopDuration * BackwardHopMultiplier;
	}
	return HopDuration;
}

bool AFrogCharacter::HasBufferedInput() const
{
	return bHasBufferedInput;
}

void AFrogCharacter::StartHop(FVector Direction)
{
	FIntPoint Delta = DirectionToGridDelta(Direction);
	FIntPoint TargetPos = GridPosition + Delta;

	bIsHopping = true;
	HopElapsed = 0.0f;
	CurrentHopDuration = GetHopDurationForDirection(Direction);
	HopStartLocation = GridToWorld(GridPosition);
	HopEndLocation = GridToWorld(TargetPos);

	// Update logical grid position immediately so collision checks use the target
	GridPosition = TargetPos;
}

void AFrogCharacter::FinishHop()
{
	bIsHopping = false;

	// Snap precisely to grid to avoid floating-point drift
	SetActorLocation(GridToWorld(GridPosition));

	OnHopCompleted.Broadcast(GridPosition);

	// Process buffered input
	if (bHasBufferedInput)
	{
		bHasBufferedInput = false;
		FVector Dir = BufferedDirection;
		BufferedDirection = FVector::ZeroVector;
		RequestHop(Dir);
	}
}

FIntPoint AFrogCharacter::DirectionToGridDelta(FVector Direction) const
{
	// Normalize to cardinal directions: largest absolute axis wins
	int32 DX = 0;
	int32 DY = 0;

	if (FMath::Abs(Direction.X) >= FMath::Abs(Direction.Y))
	{
		DX = (Direction.X > 0.0f) ? 1 : -1;
	}
	else
	{
		DY = (Direction.Y > 0.0f) ? 1 : -1;
	}

	return FIntPoint(DX, DY);
}

// -- Death and respawn ----------------------------------------------------

void AFrogCharacter::Die(EDeathType DeathType)
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	LastDeathType = DeathType;
	CurrentPlatform = nullptr;
	bIsHopping = false;

	OnFrogDied.Broadcast(DeathType);

	// Schedule respawn if the game is still running
	if (ShouldRespawn())
	{
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().SetTimer(
				RespawnTimerHandle,
				this,
				&AFrogCharacter::Respawn,
				RespawnDelay,
				false
			);
		}
	}
}

void AFrogCharacter::Respawn()
{
	bIsDead = false;
	LastDeathType = EDeathType::None;
	CurrentPlatform = nullptr;
	GridPosition = FIntPoint(6, 0);

	SetActorLocation(GridToWorld(GridPosition));
}

bool AFrogCharacter::IsOnRiverRow() const
{
	int32 Row = GridPosition.Y;
	return Row >= RiverRowMin && Row <= RiverRowMax;
}

bool AFrogCharacter::CheckRiverDeath() const
{
	if (!IsOnRiverRow())
	{
		return false;
	}

	// No platform at all
	AHazardBase* Platform = CurrentPlatform.Get();
	if (!Platform)
	{
		return true;
	}

	// Platform is submerged (turtles dive)
	if (Platform->bIsSubmerged)
	{
		return true;
	}

	return false;
}

bool AFrogCharacter::IsOffScreen() const
{
	float X = GetActorLocation().X;
	float GridWorldWidth = static_cast<float>(GridColumns) * GridCellSize;

	return X < 0.0f || X >= GridWorldWidth;
}

bool AFrogCharacter::ShouldRespawn() const
{
	return !bIsGameOver;
}

void AFrogCharacter::UpdateRiding(float DeltaTime)
{
	AHazardBase* Platform = CurrentPlatform.Get();
	if (!Platform || Platform->bIsSubmerged)
	{
		return;
	}

	float Direction = Platform->bMovesRight ? 1.0f : -1.0f;
	float DeltaX = Platform->Speed * Direction * DeltaTime;

	FVector Location = GetActorLocation();
	Location.X += DeltaX;
	SetActorLocation(Location);
}
