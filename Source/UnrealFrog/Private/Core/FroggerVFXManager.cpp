// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/FroggerVFXManager.h"
#include "Core/FlatColorMaterial.h"
#include "Core/UnrealFrogGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogFroggerVFX, Log, All);

// -- Camera-relative scaling --------------------------------------------------

float UFroggerVFXManager::CalculateScaleForScreenSize(float CameraDistance, float FOVDegrees,
	float DesiredScreenFraction, float MeshBaseDiameter)
{
	float FOVRadians = FMath::DegreesToRadians(FOVDegrees);
	float VisibleWidth = 2.0f * CameraDistance * FMath::Tan(FOVRadians * 0.5f);
	float DesiredWorldSize = VisibleWidth * DesiredScreenFraction;
	return DesiredWorldSize / FMath::Max(MeshBaseDiameter, 1.0f);
}

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

	// Compute camera-relative scale for visibility at gameplay distance
	float StartScale = 0.5f;
	float EndScale = 3.0f;
	if (APlayerCameraManager* CamMgr = UGameplayStatics::GetPlayerCameraManager(World, 0))
	{
		float CamDist = CamMgr->GetCameraLocation().Z - Location.Z;
		float FOV = CamMgr->GetFOVAngle();
		StartScale = CalculateScaleForScreenSize(CamDist, FOV, 0.08f); // 8% of screen
		EndScale = StartScale * 3.0f;
		UE_LOG(LogFroggerVFX, Warning, TEXT("DeathPuff: CamZ=%.0f FrogZ=%.0f CamDist=%.0f FOV=%.1f StartScale=%.2f EndScale=%.2f"),
			CamMgr->GetCameraLocation().Z, Location.Z, CamDist, FOV, StartScale, EndScale);
	}
	else
	{
		UE_LOG(LogFroggerVFX, Warning, TEXT("DeathPuff: NO CAMERA MANAGER — using fallback scale %.2f"), StartScale);
	}

	AActor* VFXActor = SpawnVFXActor(World, Location,
		TEXT("/Engine/BasicShapes/Sphere.Sphere"), Color, StartScale);

	if (VFXActor)
	{
		FActiveVFX Effect;
		Effect.Actor = VFXActor;
		Effect.SpawnLocation = Location;
		Effect.SpawnTime = World->GetTimeSeconds();
		Effect.Duration = 0.75f;
		Effect.StartScale = StartScale;
		Effect.EndScale = EndScale;
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

	// Compute camera-relative scale for hop dust (3% of screen)
	float DustStartScale = 0.15f;
	float DustEndScale = 0.4f;
	if (APlayerCameraManager* CamMgr = UGameplayStatics::GetPlayerCameraManager(World, 0))
	{
		float CamDist = CamMgr->GetCameraLocation().Z - Location.Z;
		float FOV = CamMgr->GetFOVAngle();
		DustStartScale = CalculateScaleForScreenSize(CamDist, FOV, 0.03f);
		DustEndScale = DustStartScale * 2.5f;
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
			TEXT("/Engine/BasicShapes/Cube.Cube"), DustColor, DustStartScale);

		if (VFXActor)
		{
			FActiveVFX Effect;
			Effect.Actor = VFXActor;
			Effect.SpawnLocation = Location + Offsets[i];
			Effect.SpawnTime = World->GetTimeSeconds();
			Effect.Duration = 0.3f;
			Effect.StartScale = DustStartScale;
			Effect.EndScale = DustEndScale;
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

	// Compute camera-relative scale for sparkle (3% of screen)
	float SparkleStartScale = 0.1f;
	float SparkleEndScale = 0.3f;
	if (APlayerCameraManager* CamMgr = UGameplayStatics::GetPlayerCameraManager(World, 0))
	{
		float CamDist = CamMgr->GetCameraLocation().Z - Location.Z;
		float FOV = CamMgr->GetFOVAngle();
		SparkleStartScale = CalculateScaleForScreenSize(CamDist, FOV, 0.03f);
		SparkleEndScale = SparkleStartScale * 2.0f;
	}

	// 4 small spheres that rise upward
	FLinearColor GoldColor(1.0f, 0.85f, 0.0f); // Golden yellow
	float Spread = 15.0f;

	for (int32 i = 0; i < 4; ++i)
	{
		float Angle = static_cast<float>(i) * PI * 0.5f;
		FVector Offset(FMath::Cos(Angle) * Spread, FMath::Sin(Angle) * Spread, 0.0);

		AActor* VFXActor = SpawnVFXActor(World, Location + Offset,
			TEXT("/Engine/BasicShapes/Sphere.Sphere"), GoldColor, SparkleStartScale);

		if (VFXActor)
		{
			FActiveVFX Effect;
			Effect.Actor = VFXActor;
			Effect.SpawnLocation = Location + Offset;
			Effect.SpawnTime = World->GetTimeSeconds();
			Effect.Duration = 1.0f;
			Effect.StartScale = SparkleStartScale;
			Effect.EndScale = SparkleEndScale;
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

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Read home slot config from GameMode — no hardcoded positions
	AUnrealFrogGameMode* GM = Cast<AUnrealFrogGameMode>(World->GetAuthGameMode());
	if (!GM)
	{
		return;
	}

	for (int32 i = 0; i < GM->HomeSlotColumns.Num(); ++i)
	{
		SpawnHomeSlotSparkle(GetHomeSlotWorldLocation(i));
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
	SpawnHomeSlotSparkle(GetHomeSlotWorldLocation(SlotIndex));
}

FVector UFroggerVFXManager::GetHomeSlotWorldLocation(int32 SlotIndex) const
{
	// Read config from GameMode — no hardcoded positions
	UWorld* World = GetWorld();
	if (World)
	{
		if (const AUnrealFrogGameMode* GM = Cast<AUnrealFrogGameMode>(World->GetAuthGameMode()))
		{
			if (GM->HomeSlotColumns.IsValidIndex(SlotIndex))
			{
				float CellSize = 100.0f; // GridCellSize from FrogCharacter default
				float Col = static_cast<float>(GM->HomeSlotColumns[SlotIndex]);
				float Row = static_cast<float>(GM->HomeSlotRow);
				return FVector(Col * CellSize, Row * CellSize, 50.0);
			}
		}
	}

	// Fallback: return origin (no world or invalid index)
	return FVector::ZeroVector;
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
