// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/UnrealFrogGameMode.h"
#include "Core/FrogCharacter.h"
#include "Core/FrogPlayerController.h"
#include "Core/HazardBase.h"
#include "Core/FroggerCameraActor.h"
#include "Core/FroggerHUD.h"
#include "Core/GroundBuilder.h"
#include "Core/LaneManager.h"
#include "Core/ScoreSubsystem.h"
#include "Core/FroggerAudioManager.h"
#include "Core/FroggerVFXManager.h"
#include "Components/LightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "TimerManager.h"
#include "EngineUtils.h"

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
	CachedLaneManager = World->SpawnActor<ALaneManager>(ALaneManager::StaticClass());
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

	// Cache subsystem pointers for use in Tick/TickTimer
	if (UGameInstance* GI = GetGameInstance())
	{
		CachedAudioManager = GI->GetSubsystem<UFroggerAudioManager>();
		CachedVFXManager = GI->GetSubsystem<UFroggerVFXManager>();
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
	if (CachedVFXManager)
	{
		CachedVFXManager->TickVFX(GetWorld()->GetTimeSeconds());
	}
}

// -- PlayUnreal ---------------------------------------------------------------

FString AUnrealFrogGameMode::GetObjectPath() const
{
	return GetPathName();
}

FString AUnrealFrogGameMode::GetPawnPath() const
{
	if (const APawn* Pawn = UGameplayStatics::GetPlayerPawn(
			const_cast<AUnrealFrogGameMode*>(this), 0))
	{
		return Pawn->GetPathName();
	}
	return FString();
}

FString AUnrealFrogGameMode::GetLaneHazardsJSON() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return TEXT("{\"hazards\":[]}");
	}

	FString HazardEntries;
	bool bFirst = true;

	for (TActorIterator<AHazardBase> It(World); It; ++It)
	{
		const AHazardBase* Hazard = *It;
		if (!Hazard)
		{
			continue;
		}

		if (!bFirst)
		{
			HazardEntries += TEXT(",");
		}
		bFirst = false;

		FVector Loc = Hazard->GetActorLocation();
		float GridCellSize = 100.0f;
		int32 Row = FMath::RoundToInt(Loc.Y / GridCellSize);

		HazardEntries += FString::Printf(
			TEXT("{\"row\":%d,\"x\":%.1f,\"speed\":%.1f,\"width\":%d,\"movesRight\":%s,\"rideable\":%s}"),
			Row,
			Loc.X,
			Hazard->Speed,
			Hazard->HazardWidthCells,
			Hazard->bMovesRight ? TEXT("true") : TEXT("false"),
			Hazard->bIsRideable ? TEXT("true") : TEXT("false"));
	}

	return FString::Printf(TEXT("{\"hazards\":[%s]}"), *HazardEntries);
}

FString AUnrealFrogGameMode::GetGameConfigJSON() const
{
	// Read live values from the player pawn, or fall back to CDO defaults
	float CellSize = 100.0f;
	float CapsuleRadius = 34.0f;
	int32 GridCols = 13;
	float HopDuration = 0.15f;
	int32 GridRowCount = 15;
	float LandingMargin = 17.0f;

	const AFrogCharacter* Frog = Cast<AFrogCharacter>(
		UGameplayStatics::GetPlayerPawn(const_cast<AUnrealFrogGameMode*>(this), 0));
	if (!Frog)
	{
		Frog = GetDefault<AFrogCharacter>();
	}
	if (Frog)
	{
		CellSize = Frog->GridCellSize;
		GridCols = Frog->GridColumns;
		HopDuration = Frog->HopDuration;
		GridRowCount = Frog->GridRows;

		if (const UCapsuleComponent* Capsule = Frog->FindComponentByClass<UCapsuleComponent>())
		{
			CapsuleRadius = Capsule->GetScaledCapsuleRadius();
		}

		LandingMargin = Frog->PlatformLandingMargin;
	}

	FString JSON = FString::Printf(
		TEXT("{\"cellSize\":%.1f,\"capsuleRadius\":%.1f,\"gridCols\":%d,\"hopDuration\":%.2f,\"platformLandingMargin\":%.1f,\"gridRowCount\":%d,\"homeRow\":%d}"),
		CellSize, CapsuleRadius, GridCols, HopDuration, LandingMargin, GridRowCount, HomeSlotRow);

	// Side effect: write to Saved/game_constants.json for offline tooling
	FString SavePath = FPaths::ProjectSavedDir() / TEXT("game_constants.json");
	FFileHelper::SaveStringToFile(JSON, *SavePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

	return JSON;
}

FString AUnrealFrogGameMode::GetGameStateJSON() const
{
	// Frog position (grid + actual world X for platform drift)
	double FrogX = 0.0;
	double FrogY = 0.0;
	double FrogWorldX = 0.0;
	if (const AFrogCharacter* Frog = Cast<AFrogCharacter>(
			UGameplayStatics::GetPlayerPawn(const_cast<AUnrealFrogGameMode*>(this), 0)))
	{
		FrogX = static_cast<double>(Frog->GridPosition.X);
		FrogY = static_cast<double>(Frog->GridPosition.Y);
		FrogWorldX = static_cast<double>(Frog->GetActorLocation().X);
	}

	// Score and lives
	int32 Score = 0;
	int32 Lives = 0;
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UScoreSubsystem* Scoring = GI->GetSubsystem<UScoreSubsystem>())
		{
			Score = Scoring->Score;
			Lives = Scoring->Lives;
		}
	}

	// State name — strip enum class prefix (e.g. "EGameState::Playing" → "Playing")
	FString StateName = UEnum::GetValueAsString(CurrentState);
	int32 ColonIndex = INDEX_NONE;
	if (StateName.FindLastChar(TEXT(':'), ColonIndex))
	{
		StateName = StateName.Mid(ColonIndex + 1);
	}

	return FString::Printf(
		TEXT("{\"score\":%d,\"lives\":%d,\"wave\":%d,\"frogPos\":[%.0f,%.0f],\"frogWorldX\":%.1f,\"gameState\":\"%s\",\"timeRemaining\":%.1f,\"homeSlotsFilledCount\":%d}"),
		Score, Lives, CurrentWave, FrogX, FrogY, FrogWorldX, *StateName, RemainingTime, HomeSlotsFilledCount);
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

	// Persist high score to disk when leaving game over screen
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UScoreSubsystem* Scoring = GI->GetSubsystem<UScoreSubsystem>())
		{
			Scoring->SaveHighScore();
		}
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

	// Persist high score to disk at game over
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UScoreSubsystem* Scoring = GI->GetSubsystem<UScoreSubsystem>())
		{
			Scoring->SaveHighScore();
		}
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
		if (CachedAudioManager)
		{
			CachedAudioManager->PlayTimerWarningSound();
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

			// TryFillHomeSlot calls OnWaveComplete() when the last slot
			// is filled, which transitions to RoundComplete and starts
			// the round-complete timer. Only start the Spawning sequence
			// if the wave is NOT complete (more slots remain).
			if (CurrentState != EGameState::RoundComplete)
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
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UScoreSubsystem* Scoring = GI->GetSubsystem<UScoreSubsystem>())
			{
				Scoring->SaveHighScore();
			}
		}
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

	// Apply wave difficulty scaling to all hazards
	if (CachedLaneManager)
	{
		CachedLaneManager->ApplyWaveDifficulty(GetSpeedMultiplier(), CurrentWave);
	}

	// Music pitch increases with wave for perceived urgency
	if (CachedAudioManager)
	{
		CachedAudioManager->SetMusicPitchMultiplier(1.0f + static_cast<float>(CurrentWave - 1) * 0.03f);
	}

	// Ground color temperature shifts with wave
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AGroundBuilder> It(World); It; ++It)
		{
			It->UpdateWaveColor(CurrentWave);
		}
	}

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
