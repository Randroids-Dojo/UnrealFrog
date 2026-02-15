// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/LaneTypes.h"
#include "Core/UnrealFrogGameMode.h"
#include "FroggerAudioManager.generated.h"

class UAudioComponent;
class USoundWave;

/** Cached PCM data for a single sound effect or music track. */
struct FCachedSoundData
{
	TArray<uint8> PCMData;
	int32 SampleRate = 44100;
	int32 NumChannels = 1;
	float Duration = 0.0f;
	bool bValid = false;
};

/**
 * Centralized audio manager for all game sound effects.
 * Loads WAV files from disk, caches PCM data, and plays them
 * via USoundWaveProcedural + PlaySound2D on demand.
 * Supports muting for CI/headless testing.
 */
UCLASS()
class UNREALFROG_API UFroggerAudioManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// -- Volume and mute controls --------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SFXVolume = 1.0f;

	/** When true, all Play methods are no-ops. Use for CI and headless testing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings")
	bool bMuted = false;

	// -- Public play interface -----------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayHopSound();

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayDeathSound(EDeathType Type);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayHomeSlotSound();

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayRoundCompleteSound();

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayGameOverSound();

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayExtraLifeSound();

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayTimerWarningSound();

	/** Load WAV files from Content/Audio/SFX/ at runtime. */
	void LoadSoundWaves();

	// -- Music interface ---------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MusicVolume = 0.7f;

	/** Load music tracks from Content/Audio/Music/ at runtime. */
	void LoadMusicTracks();

	/** Play a named music track (loops). TrackName: "Title" or "Gameplay". */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayMusic(const FString& TrackName);

	/** Stop the currently playing music. */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopMusic();

	/** Set music volume (0.0-1.0). */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetMusicVolume(float Volume);

	// -- Delegate handler adapters (UFUNCTION required for AddDynamic) -------

	UFUNCTION()
	void HandleHomeSlotFilled(int32 SlotIndex, int32 TotalFilled);

	UFUNCTION()
	void HandleExtraLife();

	UFUNCTION()
	void HandleGameOver(int32 FinalScore);

	/** Switches music track based on game state. */
	UFUNCTION()
	void HandleGameStateChanged(EGameState NewState);

	// -- Sound data references for test access -------------------------------

	UPROPERTY()
	TObjectPtr<USoundWave> HopSound;

	UPROPERTY()
	TObjectPtr<USoundWave> DeathSquishSound;

	UPROPERTY()
	TObjectPtr<USoundWave> DeathSplashSound;

	UPROPERTY()
	TObjectPtr<USoundWave> DeathOffScreenSound;

	UPROPERTY()
	TObjectPtr<USoundWave> HomeSlotSound;

	UPROPERTY()
	TObjectPtr<USoundWave> RoundCompleteSound;

	UPROPERTY()
	TObjectPtr<USoundWave> GameOverSound;

	UPROPERTY()
	TObjectPtr<USoundWave> ExtraLifeSound;

	UPROPERTY()
	TObjectPtr<USoundWave> TimerWarningSound;

	/** The persistent audio component used for music looping. */
	UPROPERTY()
	TObjectPtr<UAudioComponent> MusicComponent;

	/** Name of the currently playing music track. */
	FString CurrentMusicTrack;

private:
	// Cached PCM data for music tracks
	FCachedSoundData CachedTitleMusic;
	FCachedSoundData CachedGameplayMusic;

	// Cached PCM data for each sound
	FCachedSoundData CachedHop;
	FCachedSoundData CachedDeathSquish;
	FCachedSoundData CachedDeathSplash;
	FCachedSoundData CachedDeathOffScreen;
	FCachedSoundData CachedHomeSlot;
	FCachedSoundData CachedRoundComplete;
	FCachedSoundData CachedGameOver;
	FCachedSoundData CachedExtraLife;
	FCachedSoundData CachedTimerWarning;

	/** Play cached PCM data by creating a procedural sound wave. */
	void PlayCachedSound(FCachedSoundData& SoundData);

	/** Load a single WAV file from disk into cached PCM data. */
	bool LoadWavFromFile(const FString& FilePath, FCachedSoundData& OutData);
};
