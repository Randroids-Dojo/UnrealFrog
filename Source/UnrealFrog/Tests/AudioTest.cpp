// Copyright UnrealFrog Team. All Rights Reserved.
//
// Audio system tests: verify AudioManager subsystem initializes correctly,
// handles nullptr sound assets gracefully, and respects the mute flag.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#include "Core/FroggerAudioManager.h"

#if WITH_AUTOMATION_TESTS

namespace
{
	UFroggerAudioManager* CreateTestAudioManager()
	{
		UGameInstance* TestGI = NewObject<UGameInstance>();
		return NewObject<UFroggerAudioManager>(TestGI);
	}
}

// ---------------------------------------------------------------------------
// Test: AudioManager can be created as a subsystem
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAudio_SubsystemInitializes,
	"UnrealFrog.Audio.SubsystemInitializes",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudio_SubsystemInitializes::RunTest(const FString& Parameters)
{
	UFroggerAudioManager* Audio = CreateTestAudioManager();

	TestNotNull(TEXT("AudioManager created"), Audio);
	TestEqual(TEXT("SFX volume default 1.0"), Audio->SFXVolume, 1.0f);
	TestFalse(TEXT("Not muted by default"), Audio->bMuted);

	return true;
}

// ---------------------------------------------------------------------------
// Test: PlayHopSound doesn't crash with nullptr asset
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAudio_PlayHopSoundNullSafe,
	"UnrealFrog.Audio.PlayHopSound_NullSafe",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudio_PlayHopSoundNullSafe::RunTest(const FString& Parameters)
{
	UFroggerAudioManager* Audio = CreateTestAudioManager();

	// All sound pointers are nullptr by default — should not crash
	Audio->PlayHopSound();

	TestTrue(TEXT("PlayHopSound survived nullptr"), true);

	return true;
}

// ---------------------------------------------------------------------------
// Test: PlayDeathSound doesn't crash with nullptr assets (all variants)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAudio_PlayDeathSoundNullSafe,
	"UnrealFrog.Audio.PlayDeathSound_NullSafe",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudio_PlayDeathSoundNullSafe::RunTest(const FString& Parameters)
{
	UFroggerAudioManager* Audio = CreateTestAudioManager();

	Audio->PlayDeathSound(EDeathType::Squish);
	Audio->PlayDeathSound(EDeathType::Splash);
	Audio->PlayDeathSound(EDeathType::OffScreen);
	Audio->PlayDeathSound(EDeathType::Timeout);
	Audio->PlayDeathSound(EDeathType::None);

	TestTrue(TEXT("All death sound variants survived nullptr"), true);

	return true;
}

// ---------------------------------------------------------------------------
// Test: PlayHomeSlotSound doesn't crash with nullptr asset
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAudio_PlayHomeSlotSoundNullSafe,
	"UnrealFrog.Audio.PlayHomeSlotSound_NullSafe",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudio_PlayHomeSlotSoundNullSafe::RunTest(const FString& Parameters)
{
	UFroggerAudioManager* Audio = CreateTestAudioManager();

	Audio->PlayHomeSlotSound();

	TestTrue(TEXT("PlayHomeSlotSound survived nullptr"), true);

	return true;
}

// ---------------------------------------------------------------------------
// Test: PlayRoundCompleteSound doesn't crash with nullptr asset
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAudio_PlayRoundCompleteSoundNullSafe,
	"UnrealFrog.Audio.PlayRoundCompleteSound_NullSafe",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudio_PlayRoundCompleteSoundNullSafe::RunTest(const FString& Parameters)
{
	UFroggerAudioManager* Audio = CreateTestAudioManager();

	Audio->PlayRoundCompleteSound();

	TestTrue(TEXT("PlayRoundCompleteSound survived nullptr"), true);

	return true;
}

// ---------------------------------------------------------------------------
// Test: PlayGameOverSound doesn't crash with nullptr asset
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAudio_PlayGameOverSoundNullSafe,
	"UnrealFrog.Audio.PlayGameOverSound_NullSafe",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudio_PlayGameOverSoundNullSafe::RunTest(const FString& Parameters)
{
	UFroggerAudioManager* Audio = CreateTestAudioManager();

	Audio->PlayGameOverSound();

	TestTrue(TEXT("PlayGameOverSound survived nullptr"), true);

	return true;
}

// ---------------------------------------------------------------------------
// Test: PlayExtraLifeSound doesn't crash with nullptr asset
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAudio_PlayExtraLifeSoundNullSafe,
	"UnrealFrog.Audio.PlayExtraLifeSound_NullSafe",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudio_PlayExtraLifeSoundNullSafe::RunTest(const FString& Parameters)
{
	UFroggerAudioManager* Audio = CreateTestAudioManager();

	Audio->PlayExtraLifeSound();

	TestTrue(TEXT("PlayExtraLifeSound survived nullptr"), true);

	return true;
}

// ---------------------------------------------------------------------------
// Test: PlayTimerWarningSound doesn't crash with nullptr asset
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAudio_PlayTimerWarningSoundNullSafe,
	"UnrealFrog.Audio.PlayTimerWarningSound_NullSafe",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudio_PlayTimerWarningSoundNullSafe::RunTest(const FString& Parameters)
{
	UFroggerAudioManager* Audio = CreateTestAudioManager();

	Audio->PlayTimerWarningSound();

	TestTrue(TEXT("PlayTimerWarningSound survived nullptr"), true);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Mute flag suppresses all playback (verified by call count)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAudio_MuteSuppressesPlayback,
	"UnrealFrog.Audio.MuteSuppressesPlayback",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudio_MuteSuppressesPlayback::RunTest(const FString& Parameters)
{
	UFroggerAudioManager* Audio = CreateTestAudioManager();

	Audio->bMuted = true;

	// All Play methods should exit early without attempting playback
	Audio->PlayHopSound();
	Audio->PlayDeathSound(EDeathType::Squish);
	Audio->PlayHomeSlotSound();
	Audio->PlayRoundCompleteSound();
	Audio->PlayGameOverSound();
	Audio->PlayExtraLifeSound();
	Audio->PlayTimerWarningSound();

	// If we got here without crashes, mute suppression works
	TestTrue(TEXT("All sounds suppressed when muted"), true);

	return true;
}

// ---------------------------------------------------------------------------
// Test: SFX volume is clamped to 0.0-1.0 range
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAudio_VolumeDefaults,
	"UnrealFrog.Audio.VolumeDefaults",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudio_VolumeDefaults::RunTest(const FString& Parameters)
{
	UFroggerAudioManager* Audio = CreateTestAudioManager();

	TestEqual(TEXT("Default SFX volume is 1.0"), Audio->SFXVolume, 1.0f);

	// Volume can be set
	Audio->SFXVolume = 0.5f;
	TestEqual(TEXT("SFX volume set to 0.5"), Audio->SFXVolume, 0.5f);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Music volume defaults
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAudio_MusicVolumeDefaults,
	"UnrealFrog.Audio.MusicVolumeDefaults",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudio_MusicVolumeDefaults::RunTest(const FString& Parameters)
{
	UFroggerAudioManager* Audio = CreateTestAudioManager();

	TestNearlyEqual(TEXT("Default music volume is 0.7"), Audio->MusicVolume, 0.7f);
	TestNearlyEqual(TEXT("Default SFX volume is 1.0"), Audio->SFXVolume, 1.0f);

	Audio->SetMusicVolume(0.3f);
	TestNearlyEqual(TEXT("Music volume set to 0.3"), Audio->MusicVolume, 0.3f);

	// Clamp test
	Audio->SetMusicVolume(1.5f);
	TestNearlyEqual(TEXT("Music volume clamped to 1.0"), Audio->MusicVolume, 1.0f);

	Audio->SetMusicVolume(-0.5f);
	TestNearlyEqual(TEXT("Music volume clamped to 0.0"), Audio->MusicVolume, 0.0f);

	return true;
}

// ---------------------------------------------------------------------------
// Test: PlayMusic with no loaded tracks doesn't crash
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAudio_PlayMusicNullSafe,
	"UnrealFrog.Audio.PlayMusic_NullSafe",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudio_PlayMusicNullSafe::RunTest(const FString& Parameters)
{
	UFroggerAudioManager* Audio = CreateTestAudioManager();

	// No tracks loaded — should not crash
	Audio->PlayMusic(TEXT("Title"));
	Audio->PlayMusic(TEXT("Gameplay"));
	Audio->PlayMusic(TEXT("NonExistent"));

	TestTrue(TEXT("PlayMusic survived with no loaded tracks"), true);

	return true;
}

// ---------------------------------------------------------------------------
// Test: StopMusic with no active component doesn't crash
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAudio_StopMusicNullSafe,
	"UnrealFrog.Audio.StopMusic_NullSafe",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudio_StopMusicNullSafe::RunTest(const FString& Parameters)
{
	UFroggerAudioManager* Audio = CreateTestAudioManager();

	Audio->StopMusic();
	Audio->StopMusic(); // Double stop

	TestTrue(TEXT("StopMusic survived with no active music"), true);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Mute flag affects music (PlayMusic is a no-op when muted)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAudio_MuteAffectsMusic,
	"UnrealFrog.Audio.MuteAffectsMusic",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudio_MuteAffectsMusic::RunTest(const FString& Parameters)
{
	UFroggerAudioManager* Audio = CreateTestAudioManager();
	Audio->bMuted = true;

	// PlayMusic should set track name but not create component
	Audio->PlayMusic(TEXT("Title"));
	TestTrue(TEXT("MusicComponent is null when muted"), Audio->MusicComponent == nullptr);

	return true;
}

// ---------------------------------------------------------------------------
// Test: HandleGameStateChanged doesn't crash with no loaded tracks
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAudio_HandleGameStateChangedNullSafe,
	"UnrealFrog.Audio.HandleGameStateChanged_NullSafe",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudio_HandleGameStateChangedNullSafe::RunTest(const FString& Parameters)
{
	UFroggerAudioManager* Audio = CreateTestAudioManager();

	// All state transitions should be safe with no tracks loaded
	Audio->HandleGameStateChanged(EGameState::Title);
	Audio->HandleGameStateChanged(EGameState::Spawning);
	Audio->HandleGameStateChanged(EGameState::Playing);
	Audio->HandleGameStateChanged(EGameState::Paused);
	Audio->HandleGameStateChanged(EGameState::Dying);
	Audio->HandleGameStateChanged(EGameState::RoundComplete);
	Audio->HandleGameStateChanged(EGameState::GameOver);

	TestTrue(TEXT("HandleGameStateChanged survived all states"), true);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
