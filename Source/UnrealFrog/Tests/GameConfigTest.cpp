// Copyright UnrealFrog Team. All Rights Reserved.
//
// Tests for GetGameConfigJSON() — single source of truth for game constants.
// Sprint 11, Task #2 — Engine Architect

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Core/UnrealFrogGameMode.h"
#include "Core/FrogCharacter.h"

#if WITH_AUTOMATION_TESTS

// ===========================================================================
// GetGameConfigJSON returns valid JSON with all required fields
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameConfig_AllFieldsPresent,
	"UnrealFrog.GameConfig.AllFieldsPresent",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameConfig_AllFieldsPresent::RunTest(const FString& Parameters)
{
	AddExpectedError(TEXT("No world was found"), EAutomationExpectedErrorFlags::Contains, 0);

	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	FString JSON = GM->GetGameConfigJSON();
	UE_LOG(LogTemp, Display, TEXT("[GameConfig] JSON=%s"), *JSON);

	// All required fields must be present
	TestTrue(TEXT("Has cellSize"), JSON.Contains(TEXT("\"cellSize\":")));
	TestTrue(TEXT("Has capsuleRadius"), JSON.Contains(TEXT("\"capsuleRadius\":")));
	TestTrue(TEXT("Has gridCols"), JSON.Contains(TEXT("\"gridCols\":")));
	TestTrue(TEXT("Has hopDuration"), JSON.Contains(TEXT("\"hopDuration\":")));
	TestTrue(TEXT("Has platformLandingMargin"), JSON.Contains(TEXT("\"platformLandingMargin\":")));
	TestTrue(TEXT("Has gridRowCount"), JSON.Contains(TEXT("\"gridRowCount\":")));
	TestTrue(TEXT("Has homeRow"), JSON.Contains(TEXT("\"homeRow\":")));

	return true;
}

// ===========================================================================
// GetGameConfigJSON values match live UPROPERTY values (tuning-resilient)
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameConfig_ValuesMatchProperties,
	"UnrealFrog.GameConfig.ValuesMatchProperties",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameConfig_ValuesMatchProperties::RunTest(const FString& Parameters)
{
	AddExpectedError(TEXT("No world was found"), EAutomationExpectedErrorFlags::Contains, 0);

	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();
	AFrogCharacter* Frog = NewObject<AFrogCharacter>();

	FString JSON = GM->GetGameConfigJSON();

	// Verify values match the actual properties (read from objects, not hardcoded)
	TestTrue(TEXT("cellSize matches FrogCharacter::GridCellSize"),
		JSON.Contains(*FString::Printf(TEXT("\"cellSize\":%.1f"), Frog->GridCellSize)));

	TestTrue(TEXT("gridCols matches FrogCharacter::GridColumns"),
		JSON.Contains(*FString::Printf(TEXT("\"gridCols\":%d"), Frog->GridColumns)));

	TestTrue(TEXT("hopDuration matches FrogCharacter::HopDuration"),
		JSON.Contains(*FString::Printf(TEXT("\"hopDuration\":%.2f"), Frog->HopDuration)));

	TestTrue(TEXT("gridRowCount matches FrogCharacter::GridRows"),
		JSON.Contains(*FString::Printf(TEXT("\"gridRowCount\":%d"), Frog->GridRows)));

	TestTrue(TEXT("homeRow matches GameMode::HomeSlotRow"),
		JSON.Contains(*FString::Printf(TEXT("\"homeRow\":%d"), GM->HomeSlotRow)));

	return true;
}

// ===========================================================================
// GetGameConfigJSON all values are non-zero
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameConfig_NonZeroValues,
	"UnrealFrog.GameConfig.NonZeroValues",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameConfig_NonZeroValues::RunTest(const FString& Parameters)
{
	AddExpectedError(TEXT("No world was found"), EAutomationExpectedErrorFlags::Contains, 0);

	AUnrealFrogGameMode* GM = NewObject<AUnrealFrogGameMode>();

	FString JSON = GM->GetGameConfigJSON();

	// Check for exactly-zero values. Match ":0," or ":0}" to avoid false positives
	// on values like "0.15" (which contains ":0" as a prefix).
	auto HasZeroValue = [&JSON](const TCHAR* FieldName) -> bool {
		FString ZeroComma = FString::Printf(TEXT("\"%s\":0,"), FieldName);
		FString ZeroBrace = FString::Printf(TEXT("\"%s\":0}"), FieldName);
		return JSON.Contains(ZeroComma) || JSON.Contains(ZeroBrace);
	};

	TestFalse(TEXT("cellSize is not 0"), HasZeroValue(TEXT("cellSize")));
	TestFalse(TEXT("capsuleRadius is not 0"), HasZeroValue(TEXT("capsuleRadius")));
	TestFalse(TEXT("gridCols is not 0"), HasZeroValue(TEXT("gridCols")));
	TestFalse(TEXT("hopDuration is not 0"), HasZeroValue(TEXT("hopDuration")));
	TestFalse(TEXT("platformLandingMargin is not 0"), HasZeroValue(TEXT("platformLandingMargin")));
	TestFalse(TEXT("gridRowCount is not 0"), HasZeroValue(TEXT("gridRowCount")));
	TestFalse(TEXT("homeRow is not 0"), HasZeroValue(TEXT("homeRow")));

	return true;
}

#endif // WITH_AUTOMATION_TESTS
