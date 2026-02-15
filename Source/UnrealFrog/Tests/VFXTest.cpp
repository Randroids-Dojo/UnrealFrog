// Copyright UnrealFrog Team. All Rights Reserved.
//
// VFX system tests: verify VFXManager subsystem initializes correctly,
// handles nullptr world gracefully, and respects the disabled flag.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#include "Core/FroggerVFXManager.h"

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

#endif // WITH_AUTOMATION_TESTS
