// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/FroggerVFXManager.h"
#include "Core/FlatColorMaterial.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogFroggerVFX, Log, All);

// -- VFX spawn interface ------------------------------------------------------

void UFroggerVFXManager::SpawnDeathPuff(FVector Location, EDeathType DeathType)
{
	if (bDisabled)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Color by death type
	FLinearColor Color;
	switch (DeathType)
	{
	case EDeathType::Squish:
	case EDeathType::Timeout:
		Color = FLinearColor(1.0f, 0.2f, 0.2f); // Red
		break;
	case EDeathType::Splash:
		Color = FLinearColor(0.2f, 0.4f, 1.0f); // Blue
		break;
	case EDeathType::OffScreen:
		Color = FLinearColor(1.0f, 1.0f, 0.2f); // Yellow
		break;
	default:
		Color = FLinearColor(1.0f, 0.2f, 0.2f); // Default red
		break;
	}

	AActor* VFXActor = SpawnVFXActor(World, Location,
		TEXT("/Engine/BasicShapes/Sphere.Sphere"), Color, 0.5f);

	if (VFXActor)
	{
		FActiveVFX Effect;
		Effect.Actor = VFXActor;
		Effect.SpawnLocation = Location;
		Effect.SpawnTime = World->GetTimeSeconds();
		Effect.Duration = 0.5f;
		Effect.StartScale = 0.5f;
		Effect.EndScale = 3.0f;
		ActiveEffects.Add(Effect);
		EnforceActiveCap();
	}
}

void UFroggerVFXManager::SpawnHopDust(FVector Location)
{
	if (bDisabled)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Spawn 3 small cubes around the hop origin
	FLinearColor DustColor(0.6f, 0.5f, 0.3f); // Light brown
	float Spread = 20.0f;
	FVector Offsets[3] = {
		FVector(-Spread, 0.0, 0.0),
		FVector(Spread * 0.5, Spread * 0.866, 0.0),
		FVector(Spread * 0.5, -Spread * 0.866, 0.0)
	};

	for (int32 i = 0; i < 3; ++i)
	{
		AActor* VFXActor = SpawnVFXActor(World, Location + Offsets[i],
			TEXT("/Engine/BasicShapes/Cube.Cube"), DustColor, 0.15f);

		if (VFXActor)
		{
			FActiveVFX Effect;
			Effect.Actor = VFXActor;
			Effect.SpawnLocation = Location + Offsets[i];
			Effect.SpawnTime = World->GetTimeSeconds();
			Effect.Duration = 0.2f;
			Effect.StartScale = 0.15f;
			Effect.EndScale = 0.4f;
			ActiveEffects.Add(Effect);
		}
	}
	EnforceActiveCap();
}

void UFroggerVFXManager::SpawnHomeSlotSparkle(FVector Location)
{
	if (bDisabled)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 4 small spheres that rise upward
	FLinearColor GoldColor(1.0f, 0.85f, 0.0f); // Golden yellow
	float Spread = 15.0f;

	for (int32 i = 0; i < 4; ++i)
	{
		float Angle = static_cast<float>(i) * PI * 0.5f;
		FVector Offset(FMath::Cos(Angle) * Spread, FMath::Sin(Angle) * Spread, 0.0);

		AActor* VFXActor = SpawnVFXActor(World, Location + Offset,
			TEXT("/Engine/BasicShapes/Sphere.Sphere"), GoldColor, 0.1f);

		if (VFXActor)
		{
			FActiveVFX Effect;
			Effect.Actor = VFXActor;
			Effect.SpawnLocation = Location + Offset;
			Effect.SpawnTime = World->GetTimeSeconds();
			Effect.Duration = 1.0f;
			Effect.StartScale = 0.1f;
			Effect.EndScale = 0.3f;
			Effect.RiseVelocity = FVector(0.0, 0.0, 60.0); // Rise upward
			ActiveEffects.Add(Effect);
		}
	}
	EnforceActiveCap();
}

void UFroggerVFXManager::SpawnRoundCompleteCelebration()
{
	if (bDisabled)
	{
		return;
	}

	// Spawn sparkle at all 5 home slot positions
	// HomeSlotColumns = {1, 4, 6, 8, 11}, row 14, cell size 100
	TArray<int32> SlotColumns = {1, 4, 6, 8, 11};
	for (int32 Col : SlotColumns)
	{
		FVector SlotLocation(Col * 100.0, 14 * 100.0, 50.0);
		SpawnHomeSlotSparkle(SlotLocation);
	}
}

void UFroggerVFXManager::TickVFX(float CurrentTime)
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		FActiveVFX& Effect = ActiveEffects[i];
		AActor* Actor = Effect.Actor.Get();

		if (!Actor)
		{
			ActiveEffects.RemoveAt(i);
			continue;
		}

		float Elapsed = CurrentTime - Effect.SpawnTime;
		float Alpha = FMath::Clamp(Elapsed / Effect.Duration, 0.0f, 1.0f);

		if (Alpha >= 1.0f)
		{
			Actor->Destroy();
			ActiveEffects.RemoveAt(i);
			continue;
		}

		// Scale animation
		float CurrentScale = FMath::Lerp(Effect.StartScale, Effect.EndScale, Alpha);
		Actor->SetActorScale3D(FVector(CurrentScale));

		// Rise animation (if applicable)
		if (!Effect.RiseVelocity.IsNearlyZero())
		{
			FVector NewLocation = Effect.SpawnLocation + Effect.RiseVelocity * Elapsed;
			Actor->SetActorLocation(NewLocation);
		}
	}
}

// -- Delegate handler adapters ------------------------------------------------

void UFroggerVFXManager::HandleHomeSlotFilled(int32 SlotIndex, int32 TotalFilled)
{
	// HomeSlotColumns = {1, 4, 6, 8, 11}, cell size 100
	TArray<int32> SlotColumns = {1, 4, 6, 8, 11};
	if (SlotColumns.IsValidIndex(SlotIndex))
	{
		FVector Location(SlotColumns[SlotIndex] * 100.0, 14 * 100.0, 50.0);
		SpawnHomeSlotSparkle(Location);
	}
}

// -- Internal -----------------------------------------------------------------

AActor* UFroggerVFXManager::SpawnVFXActor(UWorld* World, FVector Location,
	const TCHAR* MeshPath, FLinearColor Color, float Scale)
{
	if (!World)
	{
		return nullptr;
	}

	AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass(),
		FTransform(FRotator::ZeroRotator, Location, FVector(Scale)));
	if (!Actor)
	{
		return nullptr;
	}

	// Add a static mesh component
	UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(Actor);
	MeshComp->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, MeshPath));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->RegisterComponent();
	Actor->SetRootComponent(MeshComp);

	// Apply flat color material
	UMaterial* FlatColor = GetOrCreateFlatColorMaterial();
	if (FlatColor)
	{
		MeshComp->SetMaterial(0, FlatColor);
		UMaterialInstanceDynamic* DynMat = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
		if (DynMat)
		{
			DynMat->SetVectorParameterValue(TEXT("Color"), Color);
		}
	}

	// Set lifespan to auto-destroy even if TickVFX isn't called
	Actor->SetLifeSpan(5.0f);

	return Actor;
}

void UFroggerVFXManager::EnforceActiveCap()
{
	while (ActiveEffects.Num() > MaxActiveVFX)
	{
		FActiveVFX& Oldest = ActiveEffects[0];
		if (AActor* Actor = Oldest.Actor.Get())
		{
			Actor->Destroy();
		}
		ActiveEffects.RemoveAt(0);
	}
}
