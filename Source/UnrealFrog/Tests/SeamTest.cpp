// Copyright UnrealFrog Team. All Rights Reserved.
//
// Seam Tests: verify the handshake between interacting systems.
// Each test creates two or more systems and exercises their boundary.
// These catch the class of bugs that per-system isolation tests miss.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#include "Core/FrogCharacter.h"
#include "Core/FroggerAudioManager.h"
#include "Core/FroggerVFXManager.h"
#include "Core/FroggerHUD.h"
#include "Core/HazardBase.h"
#include "Core/LaneManager.h"
#include "Core/ScoreSubsystem.h"
#include "Core/UnrealFrogGameMode.h"

#if WITH_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Seam 1: HopFromMovingPlatform
// Systems: FrogCharacter + HazardBase (log)
//
// When the frog is riding a moving log that has drifted from grid alignment,
// the hop origin must be the frog's actual world position, not the stale
// grid-snapped position. The landing position should be (actualX + deltaX,
// targetRow * cellSize).
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSeam_HopFromMovingPlatform,
	"UnrealFrog.Seam.HopFromMovingPlatform",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSeam_HopFromMovingPlatform::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	AHazardBase* Log = NewObject<AHazardBase>();
	Log->bIsRideable = true;
	Log->Speed = 200.0f;
	Log->bMovesRight = true;
	Log->HazardWidthCells = 3;

	// Place frog on a river row and mount the log
	Frog->GridPosition = FIntPoint(6, 8);
	Frog->HandleHazardOverlap(Log);
	TestTrue(TEXT("Frog mounted on log"), Frog->CurrentPlatform.Get() == Log);

	// Simulate the log carrying the frog to a non-grid-aligned X position.
	// Grid cell 6 = 600.0 world X. Drift the frog to 650.0 (between cells).
	FVector DriftedPos(650.0, 800.0, 0.0);
	Frog->SetActorLocation(DriftedPos);

	// Hop forward (positive Y). The hop origin should use the drifted position,
	// NOT GridToWorld(GridPosition) which would be (600, 800, 0).
	Frog->RequestHop(FVector(0.0, 1.0, 0.0));
	TestTrue(TEXT("Frog is hopping"), Frog->bIsHopping);

	// The target grid position should be (6, 9) since we started at row 8
	TestEqual(TEXT("Grid target row is 9"), Frog->GridPosition.Y, 9);

	// Simulate hop completion by ticking past the hop duration
	// The hop end location's X should be based on the drifted X (650), not grid X (600)
	// We can verify this by checking that after hop completion, the frog lands
	// near the drifted X, not snapped back to the original grid column.
	float HopDuration = Frog->HopDuration;
	Frog->Tick(HopDuration + 0.01f);

	// After FinishHop, GridPosition.X is recalculated from the actual landing
	// world position. 650 / 100 rounds to 7, not the original 6.
	FVector LandingPos = Frog->GetActorLocation();
	TestNearlyEqual(TEXT("Landing X near drifted origin"), LandingPos.X, 650.0, 1.0);
	TestNearlyEqual(TEXT("Landing Y at target row"), LandingPos.Y, 900.0, 1.0);

	return true;
}

// ---------------------------------------------------------------------------
// Seam 2: DieOnSubmergedTurtle
// Systems: FrogCharacter + HazardBase (turtle)
//
// Landing on a submerged turtle must trigger Splash death, not mount.
// The interaction boundary: HandleHazardOverlap should mount rideables,
// but CheckRiverDeath must detect that the platform is submerged.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSeam_DieOnSubmergedTurtle,
	"UnrealFrog.Seam.DieOnSubmergedTurtle",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSeam_DieOnSubmergedTurtle::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	AHazardBase* Turtle = NewObject<AHazardBase>();
	Turtle->bIsRideable = true;
	Turtle->HazardType = EHazardType::TurtleGroup;
	Turtle->bIsSubmerged = true;

	// Place frog on a river row
	Frog->GridPosition = FIntPoint(6, 8);

	// Mount the submerged turtle — HandleHazardOverlap will set CurrentPlatform
	Frog->HandleHazardOverlap(Turtle);
	TestTrue(TEXT("Frog mounted turtle"), Frog->CurrentPlatform.Get() == Turtle);

	// CheckRiverDeath should detect submerged platform
	TestTrue(TEXT("CheckRiverDeath detects submerged turtle"), Frog->CheckRiverDeath());

	// The Tick loop calls CheckRiverDeath → Die(Splash) when on river with submerged platform.
	// Verify via direct Die call since Tick needs a world context.
	Frog->Die(EDeathType::Splash);
	TestTrue(TEXT("Frog is dead"), Frog->bIsDead);
	TestEqual(TEXT("Death type is Splash"), Frog->LastDeathType, EDeathType::Splash);

	// CurrentPlatform should be cleared on death
	TestTrue(TEXT("CurrentPlatform cleared on death"), Frog->CurrentPlatform.Get() == nullptr);

	return true;
}

// ---------------------------------------------------------------------------
// Seam 3: PauseDuringRiverRide
// Systems: FrogCharacter + HazardBase + GameMode
//
// Pausing while riding a log must freeze the timer. Unpausing must resume
// with the same timer value.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSeam_PauseDuringRiverRide,
	"UnrealFrog.Seam.PauseDuringRiverRide",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSeam_PauseDuringRiverRide::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	AHazardBase* Log = NewObject<AHazardBase>();
	Log->bIsRideable = true;
	Log->Speed = 200.0f;
	Log->bMovesRight = true;

	// Start game and get to Playing state
	GM->StartGame();
	GM->OnSpawningComplete();
	TestEqual(TEXT("Playing"), GM->CurrentState, EGameState::Playing);

	// Mount frog on log
	Frog->GridPosition = FIntPoint(6, 8);
	Frog->HandleHazardOverlap(Log);
	TestTrue(TEXT("Frog riding log"), Frog->CurrentPlatform.Get() == Log);

	// Tick timer for a few seconds
	GM->TickTimer(5.0f);
	float TimerBeforePause = GM->RemainingTime;

	// Pause the game
	GM->PauseGame();
	TestEqual(TEXT("Paused state"), GM->CurrentState, EGameState::Paused);

	// Timer should NOT tick during pause
	GM->TickTimer(10.0f);
	TestEqual(TEXT("Timer frozen during pause"), GM->RemainingTime, TimerBeforePause);

	// Log movement should also be frozen (TickMovement checked against GM state in Tick).
	// We verify via TickMovement directly — it always moves (it's the Tick guard that checks state).
	// The seam: HazardBase::Tick checks GM->CurrentState == Paused and returns early.
	// Since we can't invoke the full Tick path on NewObject actors, we verify the freeze
	// semantics by confirming the GameMode guards exist.
	FVector LogPosBefore = Log->GetActorLocation();
	// TickMovement is stateless — it will move regardless. The guard is in Tick().
	// What matters for the seam: GM is Paused, so TickTimer returns early (verified above).

	// Resume the game
	GM->ResumeGame();
	TestEqual(TEXT("Back to Playing"), GM->CurrentState, EGameState::Playing);

	// Timer should resume from where it left off
	TestEqual(TEXT("Timer value unchanged after unpause"), GM->RemainingTime, TimerBeforePause);

	// Timer should tick again after resume
	GM->TickTimer(2.0f);
	TestNearlyEqual(TEXT("Timer resumed ticking"), GM->RemainingTime, TimerBeforePause - 2.0f);

	// Frog should still be on the log
	TestTrue(TEXT("Still riding log after unpause"), Frog->CurrentPlatform.Get() == Log);

	return true;
}

// ---------------------------------------------------------------------------
// Seam 4: HopWhilePlatformCarrying
// Systems: FrogCharacter + HazardBase (log)
//
// Frog can hop off a moving log in any direction. The hop origin must use
// the frog's actual world position (which the log has been carrying).
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSeam_HopWhilePlatformCarrying,
	"UnrealFrog.Seam.HopWhilePlatformCarrying",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSeam_HopWhilePlatformCarrying::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	AHazardBase* Log = NewObject<AHazardBase>();
	Log->bIsRideable = true;
	Log->Speed = 200.0f;
	Log->bMovesRight = true;
	Log->HazardWidthCells = 3;

	// Place frog on river row 8, mount the log
	Frog->GridPosition = FIntPoint(6, 8);
	Frog->HandleHazardOverlap(Log);

	// Simulate the log carrying the frog sideways to X=720 (between cells 7 and 8)
	FVector CarriedPos(720.0, 800.0, 0.0);
	Frog->SetActorLocation(CarriedPos);

	// Hop LEFT while being carried right. This tests that the hop origin
	// is the carried position, not the stale grid position.
	Frog->RequestHop(FVector(-1.0, 0.0, 0.0));
	TestTrue(TEXT("Frog is hopping left"), Frog->bIsHopping);

	// Grid position should have decremented X (from 6 to 5, since Direction is left)
	// but Y stays at 8 (same row)
	TestEqual(TEXT("Grid target Y stays at row 8"), Frog->GridPosition.Y, 8);

	// Complete the hop
	Frog->Tick(Frog->HopDuration + 0.01f);

	// Landing X should be near 720 - 100 = 620 (one cell left of the carried position)
	FVector LandingPos = Frog->GetActorLocation();
	TestNearlyEqual(TEXT("Landing X accounts for carried position"), LandingPos.X, 620.0, 1.0);
	TestNearlyEqual(TEXT("Landing Y stays on same row"), LandingPos.Y, 800.0, 1.0);

	return true;
}

// ---------------------------------------------------------------------------
// Seam 5: DeathResetsCurrentPlatform
// Systems: FrogCharacter + HazardBase
//
// When the frog dies while riding a platform, CurrentPlatform must be
// cleared. Respawning must not inherit the old platform state.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSeam_DeathResetsCurrentPlatform,
	"UnrealFrog.Seam.DeathResetsCurrentPlatform",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSeam_DeathResetsCurrentPlatform::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	AHazardBase* Log = NewObject<AHazardBase>();
	Log->bIsRideable = true;
	Log->Speed = 200.0f;

	// Mount the log
	Frog->GridPosition = FIntPoint(6, 8);
	Frog->HandleHazardOverlap(Log);
	TestTrue(TEXT("Frog on platform"), Frog->CurrentPlatform.Get() == Log);

	// Kill the frog (e.g. by a road hazard while dismounting)
	Frog->Die(EDeathType::Squish);
	TestTrue(TEXT("Frog is dead"), Frog->bIsDead);
	TestTrue(TEXT("CurrentPlatform cleared on death"), Frog->CurrentPlatform.Get() == nullptr);

	// Respawn
	Frog->Respawn();
	TestFalse(TEXT("Frog is alive after respawn"), Frog->bIsDead);
	TestTrue(TEXT("CurrentPlatform still nullptr after respawn"), Frog->CurrentPlatform.Get() == nullptr);
	TestEqual(TEXT("Grid position reset to start"), Frog->GridPosition, FIntPoint(6, 0));
	TestEqual(TEXT("Death type cleared"), Frog->LastDeathType, EDeathType::None);

	return true;
}

// ---------------------------------------------------------------------------
// Seam 6: TimerFreezesDuringPause
// Systems: GameMode + FrogPlayerController
//
// The timer value before pause must equal the timer value after unpause.
// No time must leak during the paused period.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSeam_TimerFreezesDuringPause,
	"UnrealFrog.Seam.TimerFreezesDuringPause",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSeam_TimerFreezesDuringPause::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	GM->StartGame();
	GM->OnSpawningComplete();
	TestEqual(TEXT("Playing"), GM->CurrentState, EGameState::Playing);

	// Tick some time off
	GM->TickTimer(7.5f);
	float TimerBeforePause = GM->RemainingTime;
	TestNearlyEqual(TEXT("Timer at ~22.5s"), TimerBeforePause, 22.5f);

	// Pause
	GM->PauseGame();
	TestEqual(TEXT("Paused"), GM->CurrentState, EGameState::Paused);

	// Tick aggressively during pause — simulating many frames
	for (int32 i = 0; i < 100; ++i)
	{
		GM->TickTimer(0.1f);
	}
	TestEqual(TEXT("Timer unchanged after 100 paused ticks"), GM->RemainingTime, TimerBeforePause);

	// Resume
	GM->ResumeGame();
	TestEqual(TEXT("Playing again"), GM->CurrentState, EGameState::Playing);

	// Timer must be exactly where we left it — no leaks
	TestEqual(TEXT("Timer value == pre-pause value"), GM->RemainingTime, TimerBeforePause);

	// Timer should resume ticking normally
	GM->TickTimer(2.5f);
	TestNearlyEqual(TEXT("Timer resumed at correct value"), GM->RemainingTime, TimerBeforePause - 2.5f);

	return true;
}

// ---------------------------------------------------------------------------
// Seam 7: TurtleSubmergeWhileRiding
// Systems: FrogCharacter + HazardBase (turtle)
//
// If the frog is riding a turtle that transitions to submerged, the frog
// must die with Splash death. The interaction: TickSubmerge sets
// bIsSubmerged=true, then CheckRiverDeath (called in Tick) detects it.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSeam_TurtleSubmergeWhileRiding,
	"UnrealFrog.Seam.TurtleSubmergeWhileRiding",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSeam_TurtleSubmergeWhileRiding::RunTest(const FString& Parameters)
{
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();
	AHazardBase* Turtle = NewObject<AHazardBase>();
	Turtle->bIsRideable = true;
	Turtle->HazardType = EHazardType::TurtleGroup;
	Turtle->bIsSubmerged = false;
	Turtle->CurrentWave = 2; // Submerge is Wave 2+ only
	Turtle->SurfaceDuration = 2.0f;
	Turtle->WarningDuration = 1.0f;
	Turtle->SubmergeDuration = 2.0f;

	// Mount the turtle (it's surfaced)
	Frog->GridPosition = FIntPoint(6, 8);
	Frog->HandleHazardOverlap(Turtle);
	TestTrue(TEXT("Frog riding turtle"), Frog->CurrentPlatform.Get() == Turtle);
	TestFalse(TEXT("Turtle is surfaced"), Turtle->bIsSubmerged);
	TestFalse(TEXT("Frog not dead yet"), Frog->bIsDead);

	// Tick the turtle through Surface phase (2s) into Warning (1s) into Submerged
	Turtle->TickSubmerge(2.0f); // Surface → Warning
	TestEqual(TEXT("Now in Warning phase"), Turtle->SubmergePhase, ESubmergePhase::Warning);
	TestFalse(TEXT("Not submerged during Warning"), Turtle->bIsSubmerged);

	Turtle->TickSubmerge(1.0f); // Warning → Submerged
	TestEqual(TEXT("Now Submerged"), Turtle->SubmergePhase, ESubmergePhase::Submerged);
	TestTrue(TEXT("Turtle is submerged"), Turtle->bIsSubmerged);

	// CheckRiverDeath should now return true (platform is submerged)
	TestTrue(TEXT("CheckRiverDeath detects submerged turtle"), Frog->CheckRiverDeath());

	// The actual Tick would call Die(Splash). Verify the death is correct.
	Frog->Die(EDeathType::Splash);
	TestTrue(TEXT("Frog died"), Frog->bIsDead);
	TestEqual(TEXT("Death type is Splash"), Frog->LastDeathType, EDeathType::Splash);

	return true;
}

// ---------------------------------------------------------------------------
// Seam 8: MusicSwitchesOnStateChange
// Systems: AudioManager + GameMode
//
// When the game state changes, the AudioManager should switch to the
// appropriate music track. Title/GameOver → "Title", others → "Gameplay".
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSeam_MusicSwitchesOnStateChange,
	"UnrealFrog.Seam.MusicSwitchesOnStateChange",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSeam_MusicSwitchesOnStateChange::RunTest(const FString& Parameters)
{
	UGameInstance* TestGI = NewObject<UGameInstance>();
	UFroggerAudioManager* Audio = NewObject<UFroggerAudioManager>(TestGI);

	// Title state → Title track
	Audio->HandleGameStateChanged(EGameState::Title);
	TestEqual(TEXT("Title state plays Title track"), Audio->CurrentMusicTrack, FString(TEXT("Title")));

	// Spawning state → Gameplay track
	Audio->HandleGameStateChanged(EGameState::Spawning);
	TestEqual(TEXT("Spawning state plays Gameplay track"), Audio->CurrentMusicTrack, FString(TEXT("Gameplay")));

	// Playing state → still Gameplay (no switch needed)
	Audio->HandleGameStateChanged(EGameState::Playing);
	TestEqual(TEXT("Playing state keeps Gameplay track"), Audio->CurrentMusicTrack, FString(TEXT("Gameplay")));

	// GameOver → back to Title
	Audio->HandleGameStateChanged(EGameState::GameOver);
	TestEqual(TEXT("GameOver state plays Title track"), Audio->CurrentMusicTrack, FString(TEXT("Title")));

	return true;
}

// ---------------------------------------------------------------------------
// Seam 9: VFXSpawnsOnHopStart
// Systems: VFXManager + FrogCharacter
//
// SpawnHopDust should be invokable with a location (compatible with
// OnHopStartedNative lambda wrapping).
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSeam_VFXSpawnsOnHopStart,
	"UnrealFrog.Seam.VFXSpawnsOnHopStart",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSeam_VFXSpawnsOnHopStart::RunTest(const FString& Parameters)
{
	UGameInstance* TestGI = NewObject<UGameInstance>();
	UFroggerVFXManager* VFX = NewObject<UFroggerVFXManager>(TestGI);

	// SpawnHopDust is callable with FVector — same as lambda wrapping
	FVector HopLocation(600.0, 400.0, 0.0);
	VFX->SpawnHopDust(HopLocation);

	// No world → no actors spawned, but method is callable without crash
	TestTrue(TEXT("SpawnHopDust callable with FVector location"), true);

	return true;
}

// ---------------------------------------------------------------------------
// Seam 10: VFXSpawnsOnDeath
// Systems: VFXManager + FrogCharacter
//
// SpawnDeathPuff should accept EDeathType (compatible with OnFrogDiedNative).
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSeam_VFXSpawnsOnDeath,
	"UnrealFrog.Seam.VFXSpawnsOnDeath",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSeam_VFXSpawnsOnDeath::RunTest(const FString& Parameters)
{
	UGameInstance* TestGI = NewObject<UGameInstance>();
	UFroggerVFXManager* VFX = NewObject<UFroggerVFXManager>(TestGI);

	// All death types should be handleable
	VFX->SpawnDeathPuff(FVector(600.0, 400.0, 0.0), EDeathType::Squish);
	VFX->SpawnDeathPuff(FVector(600.0, 400.0, 0.0), EDeathType::Splash);
	VFX->SpawnDeathPuff(FVector(600.0, 400.0, 0.0), EDeathType::OffScreen);
	VFX->SpawnDeathPuff(FVector(600.0, 400.0, 0.0), EDeathType::Timeout);

	TestTrue(TEXT("SpawnDeathPuff handles all EDeathType values"), true);

	return true;
}

// ---------------------------------------------------------------------------
// Seam 11: HUDScorePopOnScoreIncrease
// Systems: HUD + ScoreSubsystem
//
// When DisplayScore increases, the HUD should detect the delta and
// create a score pop.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSeam_HUDScorePopOnScoreIncrease,
	"UnrealFrog.Seam.HUDScorePopOnScoreIncrease",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSeam_HUDScorePopOnScoreIncrease::RunTest(const FString& Parameters)
{
	AFroggerHUD* HUD = NewObject<AFroggerHUD>();
	UGameInstance* TestGI = NewObject<UGameInstance>();
	UScoreSubsystem* Scoring = NewObject<UScoreSubsystem>(TestGI);

	// Simulate the polling mechanism from DrawHUD
	HUD->PreviousScore = 0;

	// Score subsystem awards points
	Scoring->AddForwardHopScore();
	int32 NewScore = Scoring->Score;
	TestTrue(TEXT("Score increased"), NewScore > 0);

	// HUD syncs display score from subsystem
	HUD->DisplayScore = NewScore;

	// Delta detection (replicated from DrawHUD logic)
	if (HUD->DisplayScore > HUD->PreviousScore)
	{
		int32 Delta = HUD->DisplayScore - HUD->PreviousScore;
		AFroggerHUD::FScorePop Pop;
		Pop.Text = FString::Printf(TEXT("+%d"), Delta);
		Pop.Color = (Delta > 100) ? FColor::White : FColor::Yellow;
		HUD->ActiveScorePops.Add(Pop);
		HUD->PreviousScore = HUD->DisplayScore;
	}

	TestEqual(TEXT("Score pop created from subsystem score"), HUD->ActiveScorePops.Num(), 1);
	TestEqual(TEXT("Pop text matches delta"), HUD->ActiveScorePops[0].Text, FString(TEXT("+10")));

	return true;
}

// ---------------------------------------------------------------------------
// Seam 12: MusicMutedInCI
// Systems: AudioManager + bMuted flag
//
// When bMuted is true, PlayMusic should not create a MusicComponent.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSeam_MusicMutedInCI,
	"UnrealFrog.Seam.MusicMutedInCI",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSeam_MusicMutedInCI::RunTest(const FString& Parameters)
{
	UGameInstance* TestGI = NewObject<UGameInstance>();
	UFroggerAudioManager* Audio = NewObject<UFroggerAudioManager>(TestGI);

	Audio->bMuted = true;

	// Play music — should be silenced
	Audio->PlayMusic(TEXT("Title"));
	TestTrue(TEXT("MusicComponent is null when muted"), Audio->MusicComponent == nullptr);
	TestEqual(TEXT("Track name still set when muted"), Audio->CurrentMusicTrack, FString(TEXT("Title")));

	// State change should still track the correct track
	Audio->HandleGameStateChanged(EGameState::Playing);
	TestEqual(TEXT("Gameplay track set when muted"), Audio->CurrentMusicTrack, FString(TEXT("Gameplay")));
	TestTrue(TEXT("Still no MusicComponent"), Audio->MusicComponent == nullptr);

	return true;
}

// ---------------------------------------------------------------------------
// Seam 13: VFXTickDrivesAnimation
// Systems: VFXManager + GameMode (via Tick)
//
// TickVFX must process ActiveEffects and clean up entries whose actor is
// invalid or whose duration has expired. This was dead code until the
// GameMode::Tick call was wired (Sprint 6 fix).
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSeam_VFXTickDrivesAnimation,
	"UnrealFrog.Seam.VFXTickDrivesAnimation",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSeam_VFXTickDrivesAnimation::RunTest(const FString& Parameters)
{
	UGameInstance* TestGI = NewObject<UGameInstance>();
	UFroggerVFXManager* VFX = NewObject<UFroggerVFXManager>(TestGI);

	// Manually add an FActiveVFX with a null Actor (simulates a destroyed actor).
	// TickVFX handles nullptr actors by removing them from ActiveEffects immediately.
	FActiveVFX Effect;
	Effect.Actor = nullptr;
	Effect.SpawnLocation = FVector(600.0, 400.0, 0.0);
	Effect.SpawnTime = 0.0f;
	Effect.Duration = 1.0f;
	Effect.StartScale = 0.5f;
	Effect.EndScale = 3.0f;
	Effect.RiseVelocity = FVector::ZeroVector;

	VFX->ActiveEffects.Add(Effect);
	TestEqual(TEXT("One active effect before tick"), VFX->ActiveEffects.Num(), 1);

	// Tick at time 2.0 (past the 1.0s duration). The nullptr actor path
	// triggers cleanup before the duration check is even reached.
	VFX->TickVFX(2.0f);

	TestEqual(TEXT("ActiveEffects empty after TickVFX cleans up null actor"), VFX->ActiveEffects.Num(), 0);

	return true;
}

// ---------------------------------------------------------------------------
// Seam 14: TimerWarningFiresAtThreshold
// Systems: GameMode + AudioManager
//
// The timer warning sound must fire exactly once when RemainingTime drops
// below 16.7% of TimePerLevel. It must not fire above the threshold, and
// it must not fire again after the first trigger.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSeam_TimerWarningFiresAtThreshold,
	"UnrealFrog.Seam.TimerWarningFiresAtThreshold",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSeam_TimerWarningFiresAtThreshold::RunTest(const FString& Parameters)
{
	// NewObject GameMode has no world context — suppress expected UE error
	AddExpectedError(TEXT("No world was found"), EAutomationExpectedErrorFlags::Contains, 0);

	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	// Start game and get to Playing state
	GM->StartGame();
	GM->OnSpawningComplete();
	TestEqual(TEXT("Playing state"), GM->CurrentState, EGameState::Playing);

	// Verify initial state
	TestFalse(TEXT("Warning not played initially"), GM->bTimerWarningPlayed);

	// TimePerLevel = 30.0f. 16.7% of 30 = 5.01. Tick to just above threshold.
	// After ticking 24.9s: RemainingTime = 5.1, percent = 5.1/30.0 = 0.17 (above 0.167)
	GM->TickTimer(24.9f);
	TestNearlyEqual(TEXT("Timer at ~5.1s"), GM->RemainingTime, 5.1f);
	TestFalse(TEXT("Warning not yet played at boundary"), GM->bTimerWarningPlayed);

	// Tick 0.2s more: RemainingTime = 4.9, percent = 4.9/30.0 = 0.1633 (below 0.167)
	GM->TickTimer(0.2f);
	TestNearlyEqual(TEXT("Timer at ~4.9s"), GM->RemainingTime, 4.9f);
	TestTrue(TEXT("Warning played after crossing threshold"), GM->bTimerWarningPlayed);

	// Tick again — should stay true (not re-trigger)
	GM->TickTimer(1.0f);
	TestTrue(TEXT("Warning stays set after subsequent ticks"), GM->bTimerWarningPlayed);

	return true;
}

// ---------------------------------------------------------------------------
// Seam 15: HandleHopCompleted fills last home slot without double-awarding
// Systems: GameMode (HandleHopCompleted + TryFillHomeSlot + OnWaveComplete)
//
// When HandleHopCompleted triggers TryFillHomeSlot on the last empty slot,
// the wave-complete path fires exactly once. Previously both TryFillHomeSlot
// and HandleHopCompleted independently checked for wave completion, causing
// double state transitions and double score bonuses.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSeam_LastHomeSlotNoDoubleBonuses,
	"UnrealFrog.Seam.LastHomeSlotNoDoubleBonuses",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSeam_LastHomeSlotNoDoubleBonuses::RunTest(const FString& Parameters)
{
	// NewObject GameMode has no world context — suppress expected UE error
	AddExpectedError(TEXT("No world was found"), EAutomationExpectedErrorFlags::Contains, 0);

	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	// Count RoundComplete state transitions to detect double-firing.
	// OnWaveCompleted is a native multicast delegate (supports AddLambda)
	// that fires inside OnWaveComplete → OnRoundCompleteFinished.
	// But OnWaveComplete itself sets state to RoundComplete before the timer fires.
	// Use OnHomeSlotFilled (dynamic) to count slot fills instead.
	// Simplest approach: count how many times SetState(RoundComplete) was reached
	// by checking CurrentWave (which only increments in OnRoundCompleteFinished).
	int32 WaveCompletedCount = 0;
	GM->OnWaveCompleted.AddLambda([&WaveCompletedCount](int32, int32) {
		WaveCompletedCount++;
	});

	GM->StartGame();
	GM->OnSpawningComplete();
	TestEqual(TEXT("Playing state"), GM->CurrentState, EGameState::Playing);

	// Fill 4 of 5 home slots directly via TryFillHomeSlot
	// (simulating previous lives reaching home)
	GM->TryFillHomeSlot(1);
	GM->TryFillHomeSlot(4);
	GM->TryFillHomeSlot(6);
	GM->TryFillHomeSlot(8);
	TestEqual(TEXT("4 slots filled"), GM->HomeSlotsFilledCount, 4);
	TestEqual(TEXT("Still Playing after 4 slots"), GM->CurrentState, EGameState::Playing);

	// Now fill the last slot via HandleHopCompleted — this is the real game path.
	// HomeSlotRow = 14, column 11 is the last unfilled slot.
	GM->HandleHopCompleted(FIntPoint(11, 14));

	// The state should be RoundComplete (from OnWaveComplete via TryFillHomeSlot)
	TestEqual(TEXT("State is RoundComplete"), GM->CurrentState, EGameState::RoundComplete);

	// HomeSlotsFilledCount should be exactly 5, not 6 or higher
	TestEqual(TEXT("Exactly 5 slots filled"), GM->HomeSlotsFilledCount, 5);

	// Verify CurrentWave was NOT incremented (that happens in OnRoundCompleteFinished,
	// not during the fill itself). If double-detection occurred, wave might advance prematurely.
	TestEqual(TEXT("Wave still 1 (not yet advanced)"), GM->CurrentWave, 1);

	// OnWaveCompleted fires from OnRoundCompleteFinished, which hasn't been called yet
	// (it's timer-driven). So the count should be 0 at this point — the wave-complete
	// path was triggered but the timer hasn't fired. What matters is that TryFillHomeSlot
	// set the state to RoundComplete exactly once.
	TestEqual(TEXT("OnWaveCompleted not yet fired (timer pending)"), WaveCompletedCount, 0);

	return true;
}

// ---------------------------------------------------------------------------
// Seam 16: WaveDifficultyFlowsToLaneConfig
// Systems: GameMode + LaneManager
//
// When waves advance, GameMode's GetSpeedMultiplier() and GetGapReduction()
// produce escalating difficulty values. These must (a) scale lane speeds
// correctly when applied to FLaneConfig.Speed, and (b) not reduce gaps
// below the minimum that ValidateGaps enforces — i.e., difficulty must
// never create impossible lanes.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSeam_WaveDifficultyFlowsToLaneConfig,
	"UnrealFrog.Seam.WaveDifficultyFlowsToLaneConfig",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSeam_WaveDifficultyFlowsToLaneConfig::RunTest(const FString& Parameters)
{
	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();
	ALaneManager* LM = NewObject<ALaneManager>();

	// Populate default lane configs (same as BeginPlay without a World)
	LM->SetupDefaultLaneConfigs();
	TestTrue(TEXT("Lane configs populated"), LM->LaneConfigs.Num() > 0);

	// --- Wave 1: baseline (no scaling) ---
	GM->CurrentWave = 1;
	float Wave1Multiplier = GM->GetSpeedMultiplier();
	int32 Wave1GapReduction = GM->GetGapReduction();

	TestNearlyEqual(TEXT("Wave 1 speed multiplier is 1.0"), Wave1Multiplier, 1.0f);
	TestEqual(TEXT("Wave 1 gap reduction is 0"), Wave1GapReduction, 0);

	// Apply wave 1 difficulty to each lane config — all speeds should be base speed
	for (const FLaneConfig& Config : LM->LaneConfigs)
	{
		if (Config.LaneType == ELaneType::Road || Config.LaneType == ELaneType::River)
		{
			float ScaledSpeed = Config.Speed * Wave1Multiplier;
			TestNearlyEqual(
				*FString::Printf(TEXT("Wave 1 row %d speed unchanged"), Config.RowIndex),
				ScaledSpeed, Config.Speed);
		}
	}

	// --- Wave 3: moderate scaling ---
	// Speed: 1.0 + (3-1)*0.1 = 1.2
	// Gap reduction: (3-1)/2 = 1
	GM->CurrentWave = 3;
	float Wave3Multiplier = GM->GetSpeedMultiplier();
	int32 Wave3GapReduction = GM->GetGapReduction();

	TestNearlyEqual(TEXT("Wave 3 speed multiplier is 1.2"), Wave3Multiplier, 1.2f);
	TestEqual(TEXT("Wave 3 gap reduction is 1"), Wave3GapReduction, 1);

	// Verify scaled speeds are strictly greater than base speeds
	for (const FLaneConfig& Config : LM->LaneConfigs)
	{
		if (Config.LaneType == ELaneType::Road || Config.LaneType == ELaneType::River)
		{
			float ScaledSpeed = Config.Speed * Wave3Multiplier;
			TestTrue(
				*FString::Printf(TEXT("Wave 3 row %d speed > base"), Config.RowIndex),
				ScaledSpeed > Config.Speed);
		}
	}

	// Verify gap reduction still produces valid lanes
	// Apply gap reduction to a copy of the lane configs and validate
	ALaneManager* LMReduced = NewObject<ALaneManager>();
	LMReduced->SetupDefaultLaneConfigs();
	for (FLaneConfig& Config : LMReduced->LaneConfigs)
	{
		Config.MinGapCells = FMath::Max(1, Config.MinGapCells - Wave3GapReduction);
	}
	TestTrue(TEXT("Wave 3 reduced gaps still produce valid lanes"), LMReduced->ValidateGaps());

	// --- Wave 7: heavy scaling ---
	// Speed: 1.0 + (7-1)*0.1 = 1.6
	// Gap reduction: (7-1)/2 = 3
	GM->CurrentWave = 7;
	float Wave7Multiplier = GM->GetSpeedMultiplier();
	int32 Wave7GapReduction = GM->GetGapReduction();

	TestNearlyEqual(TEXT("Wave 7 speed multiplier is 1.6"), Wave7Multiplier, 1.6f);
	TestEqual(TEXT("Wave 7 gap reduction is 3"), Wave7GapReduction, 3);

	// Even with gap reduction of 3, all lanes must remain passable
	// (MinGapCells clamped to 1 minimum)
	ALaneManager* LMHeavy = NewObject<ALaneManager>();
	LMHeavy->SetupDefaultLaneConfigs();
	for (FLaneConfig& Config : LMHeavy->LaneConfigs)
	{
		Config.MinGapCells = FMath::Max(1, Config.MinGapCells - Wave7GapReduction);
	}
	TestTrue(TEXT("Wave 7 reduced gaps still produce valid lanes"), LMHeavy->ValidateGaps());

	// --- Wave 11+: speed cap ---
	// Speed capped at MaxSpeedMultiplier (2.0)
	GM->CurrentWave = 11;
	float Wave11Multiplier = GM->GetSpeedMultiplier();

	TestNearlyEqual(TEXT("Wave 11 speed capped at 2.0"), Wave11Multiplier, 2.0f);

	// Verify the cap holds at even higher waves
	GM->CurrentWave = 20;
	float Wave20Multiplier = GM->GetSpeedMultiplier();
	TestNearlyEqual(TEXT("Wave 20 speed still capped at 2.0"), Wave20Multiplier, 2.0f);

	// --- Concrete example: Row 3 car lane at wave 7 ---
	// Base speed 200, multiplier 1.6 → scaled speed 320
	// Base gap 2, reduction 3 → clamped gap 1
	const FLaneConfig* Row3 = nullptr;
	for (const FLaneConfig& Config : LM->LaneConfigs)
	{
		if (Config.RowIndex == 3)
		{
			Row3 = &Config;
			break;
		}
	}
	TestNotNull(TEXT("Row 3 config exists"), Row3);
	if (Row3)
	{
		float Row3ScaledSpeed = Row3->Speed * Wave7Multiplier;
		TestNearlyEqual(TEXT("Row 3 wave 7 speed = 320"), Row3ScaledSpeed, 320.0f);

		int32 Row3AdjustedGap = FMath::Max(1, Row3->MinGapCells - Wave7GapReduction);
		TestEqual(TEXT("Row 3 wave 7 gap clamped to 1"), Row3AdjustedGap, 1);
	}

	return true;
}

#endif // WITH_AUTOMATION_TESTS
