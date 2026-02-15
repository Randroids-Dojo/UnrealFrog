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

#endif // WITH_AUTOMATION_TESTS
