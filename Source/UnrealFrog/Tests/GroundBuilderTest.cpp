// Copyright UnrealFrog Team. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Core/GroundBuilder.h"

#if WITH_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Test: AGroundBuilder can be constructed via NewObject without crashing
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGroundBuilder_Construction,
	"UnrealFrog.Ground.Construction",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGroundBuilder_Construction::RunTest(const FString& Parameters)
{
	AGroundBuilder* Builder = NewObject<AGroundBuilder>();
	TestNotNull(TEXT("GroundBuilder constructed"), Builder);
	return true;
}

// ---------------------------------------------------------------------------
// Test: CubeMesh reference is loaded in constructor
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGroundBuilder_CubeMeshLoaded,
	"UnrealFrog.Ground.CubeMeshLoaded",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGroundBuilder_CubeMeshLoaded::RunTest(const FString& Parameters)
{
	AGroundBuilder* Builder = NewObject<AGroundBuilder>();
	TestNotNull(TEXT("CubeMesh should be loaded"), Builder->CubeMesh.Get());
	return true;
}

// ---------------------------------------------------------------------------
// Test: Row configuration covers all 15 rows
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGroundBuilder_RowCount,
	"UnrealFrog.Ground.RowCount",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGroundBuilder_RowCount::RunTest(const FString& Parameters)
{
	AGroundBuilder* Builder = NewObject<AGroundBuilder>();
	TestEqual(TEXT("Should have 15 row definitions"), Builder->RowDefinitions.Num(), 15);

	// Verify each row index 0-14 is covered
	for (int32 Row = 0; Row < 15; ++Row)
	{
		bool bFound = false;
		for (const FGroundRowInfo& Info : Builder->RowDefinitions)
		{
			if (Info.Row == Row)
			{
				bFound = true;
				break;
			}
		}
		TestTrue(FString::Printf(TEXT("Row %d should be defined"), Row), bFound);
	}

	return true;
}

// ---------------------------------------------------------------------------
// Test: Home slot columns match game design spec
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGroundBuilder_HomeSlotColumns,
	"UnrealFrog.Ground.HomeSlotColumns",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGroundBuilder_HomeSlotColumns::RunTest(const FString& Parameters)
{
	AGroundBuilder* Builder = NewObject<AGroundBuilder>();
	TestEqual(TEXT("Should have 5 home slot columns"), Builder->HomeSlotColumns.Num(), 5);
	TestEqual(TEXT("Home slot 0 = column 1"), Builder->HomeSlotColumns[0], 1);
	TestEqual(TEXT("Home slot 1 = column 4"), Builder->HomeSlotColumns[1], 4);
	TestEqual(TEXT("Home slot 2 = column 6"), Builder->HomeSlotColumns[2], 6);
	TestEqual(TEXT("Home slot 3 = column 8"), Builder->HomeSlotColumns[3], 8);
	TestEqual(TEXT("Home slot 4 = column 11"), Builder->HomeSlotColumns[4], 11);
	return true;
}

// ---------------------------------------------------------------------------
// Test: Grid dimensions match spec (13x15, 100 UU cells)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGroundBuilder_GridDimensions,
	"UnrealFrog.Ground.GridDimensions",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGroundBuilder_GridDimensions::RunTest(const FString& Parameters)
{
	AGroundBuilder* Builder = NewObject<AGroundBuilder>();
	TestEqual(TEXT("GridColumns should be 13"), Builder->GridColumns, 13);
	TestEqual(TEXT("GridRows should be 15"), Builder->GridRows, 15);
	TestEqual(TEXT("GridCellSize should be 100"), Builder->GridCellSize, 100.0f);
	return true;
}

// ---------------------------------------------------------------------------
// Test: Ground color temperature shifts per wave
// Wave 1: cool (green/blue), Wave 3: warm (yellow), Wave 5: orange, Wave 7+: red
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGroundBuilder_WaveColorTemperature,
	"UnrealFrog.Ground.WaveColorTemperature",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGroundBuilder_WaveColorTemperature::RunTest(const FString& Parameters)
{
	AGroundBuilder* Builder = NewObject<AGroundBuilder>();

	// Wave 1: cool color (green/blue dominant — R should be low)
	FLinearColor Wave1Color = Builder->GetWaveColor(1);
	TestTrue(TEXT("Wave 1 is cool: blue+green > red"),
		(Wave1Color.B + Wave1Color.G) > Wave1Color.R * 2.0f);

	// Wave 3: warm color (yellow — R and G both high, B low)
	FLinearColor Wave3Color = Builder->GetWaveColor(3);
	TestTrue(TEXT("Wave 3 is warmer: R > wave 1 R"),
		Wave3Color.R > Wave1Color.R);

	// Wave 5: orange (R high, G moderate, B low)
	FLinearColor Wave5Color = Builder->GetWaveColor(5);
	TestTrue(TEXT("Wave 5 R > wave 3 R"),
		Wave5Color.R > Wave3Color.R);
	TestTrue(TEXT("Wave 5 G < wave 3 G"),
		Wave5Color.G < Wave3Color.G);

	// Wave 7+: red (R dominant, G and B low)
	FLinearColor Wave7Color = Builder->GetWaveColor(7);
	TestTrue(TEXT("Wave 7 is hot: R > G"),
		Wave7Color.R > Wave7Color.G);
	TestTrue(TEXT("Wave 7 is hot: R > B"),
		Wave7Color.R > Wave7Color.B);

	// Wave 10: should still be red (clamped at 7+)
	FLinearColor Wave10Color = Builder->GetWaveColor(10);
	TestTrue(TEXT("Wave 10 still hot"),
		Wave10Color.R > Wave10Color.G);

	// Distinct progression: each step should be visually different
	TestTrue(TEXT("Wave 1 != Wave 3 (distinct color)"),
		!Wave1Color.Equals(Wave3Color, 0.05f));
	TestTrue(TEXT("Wave 3 != Wave 5 (distinct color)"),
		!Wave3Color.Equals(Wave5Color, 0.05f));
	TestTrue(TEXT("Wave 5 != Wave 7 (distinct color)"),
		!Wave5Color.Equals(Wave7Color, 0.05f));

	return true;
}

#endif // WITH_AUTOMATION_TESTS
