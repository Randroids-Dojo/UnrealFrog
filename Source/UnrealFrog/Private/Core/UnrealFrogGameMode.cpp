// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/UnrealFrogGameMode.h"
#include "Core/FrogCharacter.h"
#include "Core/FrogPlayerController.h"
#include "Core/FroggerCameraActor.h"
#include "Core/FroggerHUD.h"
#include "Core/GroundBuilder.h"
#include "Core/LaneManager.h"
#include "Core/ScoreSubsystem.h"
#include "Core/FroggerAudioManager.h"
#include "Core/FroggerVFXManager.h"
#include "Components/LightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AUnrealFrogGameMode::AUnrealFrogGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultPawnClass = AFrogCharacter::StaticClass();
	PlayerControllerClass = AFrogPlayerController::StaticClass();
	HUDClass = AFroggerHUD::StaticClass();

	HomeSlotColumns = {1, 4, 6, 8, 11};
	ResetHomeSlots();
}

void AUnrealFrogGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Spawn support actors — the map is just an empty container
	World->SpawnActor<ALaneManager>(ALaneManager::StaticClass());
	World->SpawnActor<AGroundBuilder>(AGroundBuilder::StaticClass());
	World->SpawnActor<AFroggerCameraActor>(AFroggerCameraActor::StaticClass());

	// Spawn directional light — empty map has no lights
	FTransform SunTransform;
	SunTransform.SetRotation(FRotator(-50.0f, -30.0f, 0.0f).Quaternion());
	ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(
		ADirectionalLight::StaticClass(), SunTransform);
	if (Sun && Sun->GetLightComponent())
	{
		Sun->GetLightComponent()->SetIntensity(3.0f);
	}

	// Wire frog delegates — the default pawn is spawned by Super::BeginPlay
	AFrogCharacter* Frog = Cast<AFrogCharacter>(
		UGameplayStatics::GetPlayerPawn(this, 0));
	if (Frog)
	{
		Frog->OnHopCompleted.AddDynamic(this, &AUnrealFrogGameMode::HandleHopCompleted);
		Frog->OnFrogDied.AddDynamic(this, &AUnrealFrogGameMode::HandleFrogDied);
	}

	// Wire AudioManager to game events
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UFroggerAudioManager* Audio = GI->GetSubsystem<UFroggerAudioManager>())
		{
			Audio->LoadSoundWaves();
			Audio->LoadMusicTracks();

			// Frog events
			if (Frog)
			{
				Frog->OnHopStartedNative.AddUObject(Audio, &UFroggerAudioManager::PlayHopSound);
				Frog->OnFrogDiedNative.AddUObject(Audio, &UFroggerAudioManager::PlayDeathSound);
			}

			// GameMode events — use native delegates for lambda wiring
			OnHomeSlotFilled.AddDynamic(Audio, &UFroggerAudioManager::HandleHomeSlotFilled);

			OnWaveCompleted.AddLambda([Audio](int32, int32) {
				Audio->PlayRoundCompleteSound();
			});

			// Music: switch tracks on state transitions
			OnGameStateChanged.AddDynamic(Audio, &UFroggerAudioManager::HandleGameStateChanged);
			Audio->PlayMusic(TEXT("Title"));

			// Extra life from ScoreSubsystem
			if (UScoreSubsystem* Scoring = GI->GetSubsystem<UScoreSubsystem>())
			{
				Scoring->OnExtraLife.AddDynamic(Audio, &UFroggerAudioManager::HandleExtraLife);
				Scoring->OnGameOver.AddDynamic(Audio, &UFroggerAudioManager::HandleGameOver);
			}
		}

		// Wire VFXManager to game events
		if (UFroggerVFXManager* VFX = GI->GetSubsystem<UFroggerVFXManager>())
		{
			if (Frog)
			{
				Frog->OnHopStartedNative.AddLambda([VFX, Frog]() {
					VFX->SpawnHopDust(Frog->GetActorLocation());
				});

				Frog->OnFrogDiedNative.AddLambda([VFX, Frog](EDeathType DeathType) {
					VFX->SpawnDeathPuff(Frog->GetActorLocation(), DeathType);
				});
			}

			OnHomeSlotFilled.AddDynamic(VFX, &UFroggerVFXManager::HandleHomeSlotFilled);

			OnWaveCompleted.AddLambda([VFX](int32, int32) {
				VFX->SpawnRoundCompleteCelebration();
			});
		}
	}
}

void AUnrealFrogGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickTimer(DeltaTime);

	// Drive VFX animation
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UFroggerVFXManager* VFX = GI->GetSubsystem<UFroggerVFXManager>())
		{
			VFX->TickVFX(GetWorld()->GetTimeSeconds());
		}
	}
}

// -- State transitions --------------------------------------------------------

void AUnrealFrogGameMode::StartGame()
{
	if (CurrentState != EGameState::Title)
	{
		return;
	}

	CurrentWave = 1;
	HomeSlotsFilledCount = 0;
	HighestRowReached = 0;
	bPendingGameOver = false;
	bTimerWarningPlayed = false;
	RemainingTime = TimePerLevel;
	ResetHomeSlots();

	// Reset score and lives for the new game
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UScoreSubsystem* Scoring = GI->GetSubsystem<UScoreSubsystem>())
		{
			Scoring->StartNewGame();
		}
	}

	SetState(EGameState::Spawning);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SpawningTimerHandle, this,
			&AUnrealFrogGameMode::OnSpawningComplete,
			SpawningDuration, false);
	}
}

void AUnrealFrogGameMode::PauseGame()
{
	if (CurrentState != EGameState::Playing)
	{
		return;
	}

	SetState(EGameState::Paused);
}

void AUnrealFrogGameMode::ResumeGame()
{
	if (CurrentState != EGameState::Paused)
	{
		return;
	}

	SetState(EGameState::Playing);
}

void AUnrealFrogGameMode::ReturnToTitle()
{
	if (CurrentState != EGameState::GameOver)
	{
		return;
	}

	CurrentWave = 1;
	HomeSlotsFilledCount = 0;
	RemainingTime = TimePerLevel;
	ResetHomeSlots();
	SetState(EGameState::Title);
}

void AUnrealFrogGameMode::HandleGameOver()
{
	if (CurrentState != EGameState::Playing && CurrentState != EGameState::Paused)
	{
		return;
	}

	SetState(EGameState::GameOver);
}

// -- Home slots ---------------------------------------------------------------

bool AUnrealFrogGameMode::TryFillHomeSlot(int32 Column)
{
	int32 SlotIndex = HomeSlotColumns.IndexOfByKey(Column);
	if (SlotIndex == INDEX_NONE)
	{
		return false;
	}

	if (HomeSlots.IsValidIndex(SlotIndex) && HomeSlots[SlotIndex])
	{
		return false;
	}

	if (HomeSlots.IsValidIndex(SlotIndex))
	{
		HomeSlots[SlotIndex] = true;
	}

	HomeSlotsFilledCount++;

	OnHomeSlotFilled.Broadcast(SlotIndex, HomeSlotsFilledCount);

	if (HomeSlotsFilledCount >= TotalHomeSlots)
	{
		OnWaveComplete();
	}

	return true;
}

bool AUnrealFrogGameMode::IsHomeSlotColumn(int32 Column) const
{
	return HomeSlotColumns.Contains(Column);
}

// -- Difficulty ---------------------------------------------------------------

float AUnrealFrogGameMode::GetSpeedMultiplier() const
{
	return FMath::Min(MaxSpeedMultiplier, 1.0f + static_cast<float>(CurrentWave - 1) * DifficultySpeedIncrement);
}

int32 AUnrealFrogGameMode::GetGapReduction() const
{
	return (CurrentWave - 1) / WavesPerGapReduction;
}

// -- Timer --------------------------------------------------------------------

void AUnrealFrogGameMode::TickTimer(float DeltaTime)
{
	if (CurrentState != EGameState::Playing)
	{
		return;
	}

	RemainingTime -= DeltaTime;

	// Timer warning sound at <16.7% remaining
	float TimerPercent = RemainingTime / TimePerLevel;
	if (TimerPercent < 0.167f && !bTimerWarningPlayed)
	{
		bTimerWarningPlayed = true;
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UFroggerAudioManager* Audio = GI->GetSubsystem<UFroggerAudioManager>())
			{
				Audio->PlayTimerWarningSound();
			}
		}
	}

	if (RemainingTime <= 0.0f)
	{
		RemainingTime = 0.0f;
		OnTimerUpdate.Broadcast(RemainingTime);
		OnTimeExpired();
		return;
	}

	OnTimerUpdate.Broadcast(RemainingTime);
}

// -- Internal helpers ---------------------------------------------------------

void AUnrealFrogGameMode::ResetHomeSlots()
{
	HomeSlots.Init(false, TotalHomeSlots);
	HomeSlotsFilledCount = 0;
}

void AUnrealFrogGameMode::OnWaveComplete()
{
	// Award time bonus and round completion bonus
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UScoreSubsystem* Scoring = GI->GetSubsystem<UScoreSubsystem>())
		{
			Scoring->AddTimeBonus(RemainingTime);
			Scoring->AddBonusPoints(Scoring->RoundCompleteBonus);
			Scoring->ResetMultiplier();
		}
	}

	SetState(EGameState::RoundComplete);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RoundCompleteTimerHandle, this,
			&AUnrealFrogGameMode::OnRoundCompleteFinished,
			RoundCompleteDuration, false);
	}
}

void AUnrealFrogGameMode::OnTimeExpired()
{
	OnTimerExpired.Broadcast();
	HandleFrogDied(EDeathType::Timeout);
}

void AUnrealFrogGameMode::SetState(EGameState NewState)
{
	CurrentState = NewState;
	OnGameStateChanged.Broadcast(NewState);
}

// -- Orchestration --------------------------------------------------------

void AUnrealFrogGameMode::HandleFrogDied(EDeathType DeathType)
{
	if (CurrentState != EGameState::Playing)
	{
		return;
	}

	// Update scoring: lose a life, check for game over
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UScoreSubsystem* Scoring = GI->GetSubsystem<UScoreSubsystem>())
		{
			Scoring->LoseLife();
			if (Scoring->IsGameOver())
			{
				bPendingGameOver = true;
			}
		}
	}

	SetState(EGameState::Dying);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DyingTimerHandle, this,
			&AUnrealFrogGameMode::OnDyingComplete,
			DyingDuration, false);
	}
}

void AUnrealFrogGameMode::HandleHopCompleted(FIntPoint NewGridPosition)
{
	if (CurrentState != EGameState::Playing)
	{
		return;
	}

	int32 NewRow = NewGridPosition.Y;

	// Forward hop scoring: only score if reaching a new highest row
	if (NewRow > HighestRowReached)
	{
		HighestRowReached = NewRow;

		if (UGameInstance* GI = GetGameInstance())
		{
			if (UScoreSubsystem* Scoring = GI->GetSubsystem<UScoreSubsystem>())
			{
				Scoring->AddForwardHopScore();
			}
		}
	}

	// Check if frog reached the home slot row
	if (NewRow == HomeSlotRow)
	{
		int32 Column = NewGridPosition.X;
		if (TryFillHomeSlot(Column))
		{
			HighestRowReached = 0;

			if (UGameInstance* GI = GetGameInstance())
			{
				if (UScoreSubsystem* Scoring = GI->GetSubsystem<UScoreSubsystem>())
				{
					Scoring->AddHomeSlotScore();
					Scoring->ResetMultiplier();
				}
			}

			// Check if all home slots are filled → round complete
			if (HomeSlotsFilledCount >= HomeSlotColumns.Num())
			{
				SetState(EGameState::RoundComplete);

				if (UWorld* World = GetWorld())
				{
					World->GetTimerManager().SetTimer(
						RoundCompleteTimerHandle, this,
						&AUnrealFrogGameMode::OnRoundCompleteFinished,
						RoundCompleteDuration, false);
				}
			}
			else
			{
				// Slot filled but more to go — respawn frog at start
				SetState(EGameState::Spawning);

				if (UWorld* World = GetWorld())
				{
					World->GetTimerManager().SetTimer(
						SpawningTimerHandle, this,
						&AUnrealFrogGameMode::OnSpawningComplete,
						SpawningDuration, false);
				}
			}
		}
		else
		{
			// Not a valid home slot column — death (landed in the bushes)
			HandleFrogDied(EDeathType::Squish);
		}
	}
}

void AUnrealFrogGameMode::OnSpawningComplete()
{
	if (CurrentState != EGameState::Spawning)
	{
		return;
	}

	// Always reset frog to start position — whether it died (respawn) or
	// successfully filled a home slot (alive but at the top of the screen)
	if (AFrogCharacter* Frog = Cast<AFrogCharacter>(
			UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		Frog->Respawn();
	}

	SetState(EGameState::Playing);
}

void AUnrealFrogGameMode::OnDyingComplete()
{
	if (CurrentState != EGameState::Dying)
	{
		return;
	}

	if (bPendingGameOver)
	{
		SetState(EGameState::GameOver);
	}
	else
	{
		HighestRowReached = 0;
		SetState(EGameState::Spawning);

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				SpawningTimerHandle, this,
				&AUnrealFrogGameMode::OnSpawningComplete,
				SpawningDuration, false);
		}
	}
}

void AUnrealFrogGameMode::OnRoundCompleteFinished()
{
	if (CurrentState != EGameState::RoundComplete)
	{
		return;
	}

	int32 CompletedWave = CurrentWave;
	CurrentWave++;

	ResetHomeSlots();
	HighestRowReached = 0;
	RemainingTime = TimePerLevel;
	bTimerWarningPlayed = false;

	OnWaveCompleted.Broadcast(CompletedWave, CurrentWave);
	OnWaveCompletedBP.Broadcast(CompletedWave, CurrentWave);

	SetState(EGameState::Spawning);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SpawningTimerHandle, this,
			&AUnrealFrogGameMode::OnSpawningComplete,
			SpawningDuration, false);
	}
}
