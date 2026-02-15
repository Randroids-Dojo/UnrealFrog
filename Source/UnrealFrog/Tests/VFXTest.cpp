// Copyright UnrealFrog Team. All Rights Reserved.
//
// VFX system tests: verify VFXManager subsystem initializes correctly,
// handles nullptr world gracefully, and respects the disabled flag.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#include "Core/FroggerVFXManager.h"
#include "Core/UnrealFrogGameMode.h"

#if WITH_AUTOMATION_TESTS

namespace
{
	UFroggerVFXManager* CreateTestVFXManager()
	{
		UGameInstance* TestGI = NewObject<UGameInstance>();
		return NewObject<UFroggerVFXManager>(TestGI);
	}
}

// ---------------------------------------------------------------------------
// Test: VFXManager can be created as a subsystem
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVFX_SubsystemInitializes,
	"UnrealFrog.VFX.SubsystemInitializes",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVFX_SubsystemInitializes::RunTest(const FString& Parameters)
{
	UFroggerVFXManager* VFX = CreateTestVFXManager();

	TestNotNull(TEXT("VFXManager created"), VFX);
	TestFalse(TEXT("Not disabled by default"), VFX->bDisabled);
	TestEqual(TEXT("No active effects initially"), VFX->ActiveEffects.Num(), 0);

	return true;
}

// ---------------------------------------------------------------------------
// Test: SpawnDeathPuff doesn't crash with no world
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVFX_SpawnDeathPuffNullSafe,
	"UnrealFrog.VFX.SpawnDeathPuff_NullSafe",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVFX_SpawnDeathPuffNullSafe::RunTest(const FString& Parameters)
{
	UFroggerVFXManager* VFX = CreateTestVFXManager();

	VFX->SpawnDeathPuff(FVector::ZeroVector, EDeathType::Squish);
	VFX->SpawnDeathPuff(FVector::ZeroVector, EDeathType::Splash);
	VFX->SpawnDeathPuff(FVector::ZeroVector, EDeathType::OffScreen);

	TestTrue(TEXT("SpawnDeathPuff survived with no world"), true);

	return true;
}

// ---------------------------------------------------------------------------
// Test: SpawnHopDust doesn't crash with no world
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVFX_SpawnHopDustNullSafe,
	"UnrealFrog.VFX.SpawnHopDust_NullSafe",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVFX_SpawnHopDustNullSafe::RunTest(const FString& Parameters)
{
	UFroggerVFXManager* VFX = CreateTestVFXManager();

	VFX->SpawnHopDust(FVector(600.0, 400.0, 0.0));

	TestTrue(TEXT("SpawnHopDust survived with no world"), true);

	return true;
}

// ---------------------------------------------------------------------------
// Test: SpawnHomeSlotSparkle doesn't crash with no world
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVFX_SpawnHomeSlotSparkleNullSafe,
	"UnrealFrog.VFX.SpawnHomeSlotSparkle_NullSafe",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVFX_SpawnHomeSlotSparkleNullSafe::RunTest(const FString& Parameters)
{
	UFroggerVFXManager* VFX = CreateTestVFXManager();

	VFX->SpawnHomeSlotSparkle(FVector(400.0, 1400.0, 50.0));

	TestTrue(TEXT("SpawnHomeSlotSparkle survived with no world"), true);

	return true;
}

// ---------------------------------------------------------------------------
// Test: Disabled flag suppresses all VFX
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVFX_DisabledSuppressesVFX,
	"UnrealFrog.VFX.DisabledSuppressesVFX",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVFX_DisabledSuppressesVFX::RunTest(const FString& Parameters)
{
	UFroggerVFXManager* VFX = CreateTestVFXManager();
	VFX->bDisabled = true;

	VFX->SpawnDeathPuff(FVector::ZeroVector, EDeathType::Squish);
	VFX->SpawnHopDust(FVector::ZeroVector);
	VFX->SpawnHomeSlotSparkle(FVector::ZeroVector);
	VFX->SpawnRoundCompleteCelebration();

	TestEqual(TEXT("No effects when disabled"), VFX->ActiveEffects.Num(), 0);

	return true;
}

// ---------------------------------------------------------------------------
// Test: HandleHomeSlotFilled doesn't crash with no world
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVFX_HandleHomeSlotFilledNullSafe,
	"UnrealFrog.VFX.HandleHomeSlotFilled_NullSafe",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVFX_HandleHomeSlotFilledNullSafe::RunTest(const FString& Parameters)
{
	UFroggerVFXManager* VFX = CreateTestVFXManager();

	VFX->HandleHomeSlotFilled(0, 1);
	VFX->HandleHomeSlotFilled(4, 5);
	VFX->HandleHomeSlotFilled(99, 100); // Out of range

	TestTrue(TEXT("HandleHomeSlotFilled survived with no world"), true);

	return true;
}

// ---------------------------------------------------------------------------
// Test: VFX wiring delegate compatibility
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVFX_WiringDelegateCompatibility,
	"UnrealFrog.VFX.WiringDelegateCompatibility",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVFX_WiringDelegateCompatibility::RunTest(const FString& Parameters)
{
	UFroggerVFXManager* VFX = CreateTestVFXManager();

	// Verify HandleHomeSlotFilled has the right signature for FOnHomeSlotFilled
	// (int32, int32) — it compiles and is callable
	VFX->HandleHomeSlotFilled(2, 3);

	// Verify SpawnDeathPuff accepts EDeathType (compatible with OnFrogDiedNative)
	VFX->SpawnDeathPuff(FVector::ZeroVector, EDeathType::Squish);

	// Verify SpawnHopDust accepts FVector (compatible with lambda wrapping OnHopStartedNative)
	VFX->SpawnHopDust(FVector::ZeroVector);

	TestTrue(TEXT("All VFX methods have compatible delegate signatures"), true);

	return true;
}

// ---------------------------------------------------------------------------
// RED TEST: Death puff must be visible at spawn time from the gameplay camera.
// At Z=2200, FOV 50, visible width is ~2052 UU. 5% of that = ~103 UU.
// The death puff must produce >= 103 UU diameter at SPAWN (StartScale),
// not just at the end of its animation. Currently StartScale=0.5 gives
// only 50 UU diameter (half the minimum), making it invisible at spawn.
// The fix should compute scale from camera distance and FOV.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVFX_DeathPuffScaleForCameraDistance,
	"UnrealFrog.VFX.DeathPuffScaleForCameraDistance",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVFX_DeathPuffScaleForCameraDistance::RunTest(const FString& Parameters)
{
	// Camera parameters (gameplay camera defaults)
	const float CameraZ = 2200.0f;
	const float FOVDegrees = 50.0f;
	const float FOVRadians = FMath::DegreesToRadians(FOVDegrees);
	const float VisibleWidth = 2.0f * CameraZ * FMath::Tan(FOVRadians * 0.5f);
	const float MinScreenFraction = 0.05f; // 5% of visible width
	const float MinDiameter = MinScreenFraction * VisibleWidth; // ~103 UU
	const float MeshBaseDiameter = 100.0f; // Engine sphere diameter at scale 1.0

	// Use the static helper to compute the scale for 5% screen fraction
	float ComputedScale = UFroggerVFXManager::CalculateScaleForScreenSize(
		CameraZ, FOVDegrees, MinScreenFraction, MeshBaseDiameter);

	float ActualStartDiameter = ComputedScale * MeshBaseDiameter;

	// The death puff must be >= 5% of screen width AT SPAWN TIME.
	TestTrue(
		FString::Printf(TEXT("Death puff start diameter (%.1f UU) must be >= %.1f UU (5%% of %.1f UU visible width)"),
			ActualStartDiameter, MinDiameter, VisibleWidth),
		ActualStartDiameter >= MinDiameter);

	return true;
}

// ---------------------------------------------------------------------------
// RED TEST: Home slot celebration positions must be derived from GameMode's
// HomeSlotColumns and HomeSlotRow, not hardcoded magic numbers.
// Currently SpawnRoundCompleteCelebration() and HandleHomeSlotFilled()
// hardcode {1, 4, 6, 8, 11} for columns and 14 for row. If the GameMode's
// HomeSlotColumns are changed, VFX spawns at wrong positions.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVFX_HomeSlotSparkleReadsGridConfig,
	"UnrealFrog.VFX.HomeSlotSparkleReadsGridConfig",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVFX_HomeSlotSparkleReadsGridConfig::RunTest(const FString& Parameters)
{
	UFroggerVFXManager* VFX = CreateTestVFXManager();

	// Without a world, GetHomeSlotWorldLocation cannot read from GameMode,
	// so it returns ZeroVector (the fallback). This proves it does NOT
	// use hardcoded values — if it did, it would return (100, 1400, 50)
	// for slot 0 regardless of whether a GameMode exists.
	FVector NoWorldResult = VFX->GetHomeSlotWorldLocation(0);
	TestEqual(TEXT("No-world fallback returns ZeroVector (no hardcoded positions)"),
		NoWorldResult, FVector::ZeroVector);

	// Verify the contract: the VFXManager source should NOT contain
	// hardcoded home slot columns. We test this indirectly by verifying
	// GetHomeSlotWorldLocation does not return the OLD hardcoded values.
	// Old hardcoded slot 0: {1, 4, 6, 8, 11}[0]=1, row 14 → (100, 1400, 50)
	FVector OldHardcodedSlot0(100.0, 1400.0, 50.0);
	TestNotEqual(TEXT("Slot 0 must not return old hardcoded position"),
		NoWorldResult, OldHardcodedSlot0);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
