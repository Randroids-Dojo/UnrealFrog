// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/UnrealFrogGameMode.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogFrogGameAudio, Log, All);

AUnrealFrogGameMode::AUnrealFrogGameMode()
{
	HomeSlotColumns = {1, 4, 6, 8, 11};
	ResetHomeSlots();
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
	RemainingTime = TimePerLevel;
	ResetHomeSlots();
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
		// Scoring will be wired to ScoreSubsystem in BeginPlay (with world context)
	}

	// Check if frog reached the home slot row
	if (NewRow == HomeSlotRow)
	{
		int32 Column = NewGridPosition.X;
		if (TryFillHomeSlot(Column))
		{
			// Home slot filled — frog resets, scoring handled by wiring
			HighestRowReached = 0;
		}
		// If not a valid home slot column, frog just sits on the goal row
	}
}

void AUnrealFrogGameMode::OnSpawningComplete()
{
	if (CurrentState != EGameState::Spawning)
	{
		return;
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

// -- Audio stubs ----------------------------------------------------------

void AUnrealFrogGameMode::PlayRoundCompleteSound()
{
	UE_LOG(LogFrogGameAudio, Verbose, TEXT("PlayRoundCompleteSound: stub"));
}

void AUnrealFrogGameMode::PlayGameOverSound()
{
	UE_LOG(LogFrogGameAudio, Verbose, TEXT("PlayGameOverSound: stub"));
}
