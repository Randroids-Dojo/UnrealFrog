// Copyright UnrealFrog Team. All Rights Reserved.

#include "Core/FroggerAudioManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Sound/SoundWaveProcedural.h"

DEFINE_LOG_CATEGORY_STATIC(LogFroggerAudio, Log, All);

void UFroggerAudioManager::PlayHopSound()
{
	PlayCachedSound(CachedHop);
}

void UFroggerAudioManager::PlayDeathSound(EDeathType Type)
{
	switch (Type)
	{
	case EDeathType::Squish:
	case EDeathType::Timeout:
		PlayCachedSound(CachedDeathSquish);
		break;
	case EDeathType::Splash:
		PlayCachedSound(CachedDeathSplash);
		break;
	case EDeathType::OffScreen:
		PlayCachedSound(CachedDeathOffScreen);
		break;
	default:
		break;
	}
}

void UFroggerAudioManager::PlayHomeSlotSound()
{
	PlayCachedSound(CachedHomeSlot);
}

void UFroggerAudioManager::PlayRoundCompleteSound()
{
	PlayCachedSound(CachedRoundComplete);
}

void UFroggerAudioManager::PlayGameOverSound()
{
	PlayCachedSound(CachedGameOver);
}

void UFroggerAudioManager::PlayExtraLifeSound()
{
	PlayCachedSound(CachedExtraLife);
}

void UFroggerAudioManager::PlayTimerWarningSound()
{
	PlayCachedSound(CachedTimerWarning);
}

// -- Delegate handler adapters ------------------------------------------------

void UFroggerAudioManager::HandleHomeSlotFilled(int32 SlotIndex, int32 TotalFilled)
{
	PlayHomeSlotSound();
}

void UFroggerAudioManager::HandleExtraLife()
{
	PlayExtraLifeSound();
}

void UFroggerAudioManager::HandleGameOver(int32 FinalScore)
{
	PlayGameOverSound();
}

// -- Sound loading ------------------------------------------------------------

void UFroggerAudioManager::LoadSoundWaves()
{
	FString AudioDir = FPaths::ProjectContentDir() / TEXT("Audio/SFX");

	LoadWavFromFile(AudioDir / TEXT("SFX_Hop.wav"), CachedHop);
	LoadWavFromFile(AudioDir / TEXT("SFX_DeathSquish.wav"), CachedDeathSquish);
	LoadWavFromFile(AudioDir / TEXT("SFX_DeathSplash.wav"), CachedDeathSplash);
	LoadWavFromFile(AudioDir / TEXT("SFX_DeathOffScreen.wav"), CachedDeathOffScreen);
	LoadWavFromFile(AudioDir / TEXT("SFX_HomeSlot.wav"), CachedHomeSlot);
	LoadWavFromFile(AudioDir / TEXT("SFX_RoundComplete.wav"), CachedRoundComplete);
	LoadWavFromFile(AudioDir / TEXT("SFX_GameOver.wav"), CachedGameOver);
	LoadWavFromFile(AudioDir / TEXT("SFX_ExtraLife.wav"), CachedExtraLife);
	LoadWavFromFile(AudioDir / TEXT("SFX_TimerWarning.wav"), CachedTimerWarning);

	// Also set the USoundWave* pointers for test compatibility (nullptr-safe checks)
	HopSound = CachedHop.bValid ? NewObject<USoundWave>(this) : nullptr;
	DeathSquishSound = CachedDeathSquish.bValid ? NewObject<USoundWave>(this) : nullptr;
	DeathSplashSound = CachedDeathSplash.bValid ? NewObject<USoundWave>(this) : nullptr;
	DeathOffScreenSound = CachedDeathOffScreen.bValid ? NewObject<USoundWave>(this) : nullptr;
	HomeSlotSound = CachedHomeSlot.bValid ? NewObject<USoundWave>(this) : nullptr;
	RoundCompleteSound = CachedRoundComplete.bValid ? NewObject<USoundWave>(this) : nullptr;
	GameOverSound = CachedGameOver.bValid ? NewObject<USoundWave>(this) : nullptr;
	ExtraLifeSound = CachedExtraLife.bValid ? NewObject<USoundWave>(this) : nullptr;
	TimerWarningSound = CachedTimerWarning.bValid ? NewObject<USoundWave>(this) : nullptr;

	int32 Loaded = 0;
	if (CachedHop.bValid) Loaded++;
	if (CachedDeathSquish.bValid) Loaded++;
	if (CachedDeathSplash.bValid) Loaded++;
	if (CachedDeathOffScreen.bValid) Loaded++;
	if (CachedHomeSlot.bValid) Loaded++;
	if (CachedRoundComplete.bValid) Loaded++;
	if (CachedGameOver.bValid) Loaded++;
	if (CachedExtraLife.bValid) Loaded++;
	if (CachedTimerWarning.bValid) Loaded++;

	UE_LOG(LogFroggerAudio, Log, TEXT("LoadSoundWaves: loaded %d/9 sounds from %s"), Loaded, *AudioDir);
}

bool UFroggerAudioManager::LoadWavFromFile(const FString& FilePath, FCachedSoundData& OutData)
{
	TArray<uint8> RawFileData;
	if (!FFileHelper::LoadFileToArray(RawFileData, *FilePath))
	{
		UE_LOG(LogFroggerAudio, Warning, TEXT("LoadWavFromFile: could not read '%s'"), *FilePath);
		OutData.bValid = false;
		return false;
	}

	if (RawFileData.Num() < 44)
	{
		UE_LOG(LogFroggerAudio, Warning, TEXT("LoadWavFromFile: file too small '%s'"), *FilePath);
		OutData.bValid = false;
		return false;
	}

	if (RawFileData[0] != 'R' || RawFileData[1] != 'I' || RawFileData[2] != 'F' || RawFileData[3] != 'F')
	{
		UE_LOG(LogFroggerAudio, Warning, TEXT("LoadWavFromFile: not a RIFF file '%s'"), *FilePath);
		OutData.bValid = false;
		return false;
	}

	OutData.NumChannels = RawFileData[22] | (RawFileData[23] << 8);
	OutData.SampleRate = RawFileData[24] | (RawFileData[25] << 8) | (RawFileData[26] << 16) | (RawFileData[27] << 24);
	int32 BitsPerSample = RawFileData[34] | (RawFileData[35] << 8);

	// Find the 'data' chunk
	int32 DataOffset = -1;
	int32 DataSize = 0;
	for (int32 i = 12; i < RawFileData.Num() - 8; ++i)
	{
		if (RawFileData[i] == 'd' && RawFileData[i + 1] == 'a' && RawFileData[i + 2] == 't' && RawFileData[i + 3] == 'a')
		{
			DataSize = RawFileData[i + 4] | (RawFileData[i + 5] << 8) | (RawFileData[i + 6] << 16) | (RawFileData[i + 7] << 24);
			DataOffset = i + 8;
			break;
		}
	}

	if (DataOffset < 0 || DataOffset + DataSize > RawFileData.Num())
	{
		UE_LOG(LogFroggerAudio, Warning, TEXT("LoadWavFromFile: could not find data chunk in '%s'"), *FilePath);
		OutData.bValid = false;
		return false;
	}

	OutData.PCMData.SetNumUninitialized(DataSize);
	FMemory::Memcpy(OutData.PCMData.GetData(), RawFileData.GetData() + DataOffset, DataSize);
	OutData.Duration = static_cast<float>(DataSize) / static_cast<float>(OutData.SampleRate * OutData.NumChannels * (BitsPerSample / 8));
	OutData.bValid = true;

	UE_LOG(LogFroggerAudio, Log, TEXT("Loaded '%s': %dHz %dch %dbit %.2fs (%d PCM bytes)"),
		*FilePath, OutData.SampleRate, OutData.NumChannels, BitsPerSample, OutData.Duration, DataSize);

	return true;
}

// -- Music playback -----------------------------------------------------------

void UFroggerAudioManager::LoadMusicTracks()
{
	FString MusicDir = FPaths::ProjectContentDir() / TEXT("Audio/Music");

	LoadWavFromFile(MusicDir / TEXT("Music_Title.wav"), CachedTitleMusic);
	LoadWavFromFile(MusicDir / TEXT("Music_Gameplay.wav"), CachedGameplayMusic);

	int32 Loaded = 0;
	if (CachedTitleMusic.bValid) Loaded++;
	if (CachedGameplayMusic.bValid) Loaded++;

	UE_LOG(LogFroggerAudio, Log, TEXT("LoadMusicTracks: loaded %d/2 tracks from %s"), Loaded, *MusicDir);
}

void UFroggerAudioManager::PlayMusic(const FString& TrackName)
{
	if (bMuted)
	{
		CurrentMusicTrack = TrackName;
		return;
	}

	FCachedSoundData* TrackData = nullptr;
	if (TrackName == TEXT("Title"))
	{
		TrackData = &CachedTitleMusic;
	}
	else if (TrackName == TEXT("Gameplay"))
	{
		TrackData = &CachedGameplayMusic;
	}

	if (!TrackData || !TrackData->bValid || TrackData->PCMData.Num() == 0)
	{
		CurrentMusicTrack = TrackName;
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		CurrentMusicTrack = TrackName;
		return;
	}

	// Stop current music if playing
	StopMusic();

	// Create a procedural sound wave with the music data
	USoundWaveProcedural* ProceduralWave = NewObject<USoundWaveProcedural>(this);
	ProceduralWave->SetSampleRate(TrackData->SampleRate);
	ProceduralWave->NumChannels = TrackData->NumChannels;
	ProceduralWave->Duration = TrackData->Duration;
	ProceduralWave->SoundGroup = SOUNDGROUP_Music;
	ProceduralWave->bLooping = true;

	ProceduralWave->QueueAudio(TrackData->PCMData.GetData(), TrackData->PCMData.Num());

	// Create a persistent audio component for looping
	MusicComponent = UGameplayStatics::SpawnSound2D(World, ProceduralWave, MusicVolume);
	CurrentMusicTrack = TrackName;

	UE_LOG(LogFroggerAudio, Log, TEXT("PlayMusic: playing '%s' (%.1fs)"), *TrackName, TrackData->Duration);
}

void UFroggerAudioManager::StopMusic()
{
	if (MusicComponent)
	{
		MusicComponent->Stop();
		MusicComponent->DestroyComponent();
		MusicComponent = nullptr;
	}
	CurrentMusicTrack = TEXT("");
}

void UFroggerAudioManager::SetMusicVolume(float Volume)
{
	MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	if (MusicComponent)
	{
		MusicComponent->SetVolumeMultiplier(MusicVolume);
	}
}

void UFroggerAudioManager::SetMusicPitchMultiplier(float Multiplier)
{
	MusicPitchMultiplier = FMath::Max(0.1f, Multiplier);
	if (MusicComponent)
	{
		MusicComponent->SetPitchMultiplier(MusicPitchMultiplier);
	}
}

void UFroggerAudioManager::HandleGameStateChanged(EGameState NewState)
{
	switch (NewState)
	{
	case EGameState::Title:
	case EGameState::GameOver:
		if (CurrentMusicTrack != TEXT("Title"))
		{
			PlayMusic(TEXT("Title"));
		}
		break;
	case EGameState::Spawning:
	case EGameState::Playing:
	case EGameState::Paused:
	case EGameState::Dying:
	case EGameState::RoundComplete:
		if (CurrentMusicTrack != TEXT("Gameplay"))
		{
			PlayMusic(TEXT("Gameplay"));
		}
		break;
	}
}

// -- Internal -----------------------------------------------------------------

void UFroggerAudioManager::PlayCachedSound(FCachedSoundData& SoundData)
{
	if (bMuted)
	{
		return;
	}

	if (!SoundData.bValid || SoundData.PCMData.Num() == 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Create a fresh procedural sound wave and queue the PCM data into it
	USoundWaveProcedural* ProceduralWave = NewObject<USoundWaveProcedural>(this);
	ProceduralWave->SetSampleRate(SoundData.SampleRate);
	ProceduralWave->NumChannels = SoundData.NumChannels;
	ProceduralWave->Duration = SoundData.Duration;
	ProceduralWave->SoundGroup = SOUNDGROUP_Effects;
	ProceduralWave->bLooping = false;

	// Queue the raw PCM data — the procedural wave's GeneratePCMData will drain it
	ProceduralWave->QueueAudio(SoundData.PCMData.GetData(), SoundData.PCMData.Num());

	UGameplayStatics::PlaySound2D(World, ProceduralWave, SFXVolume);
}
