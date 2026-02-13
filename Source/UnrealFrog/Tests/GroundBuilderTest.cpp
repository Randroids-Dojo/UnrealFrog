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

#endif // WITH_AUTOMATION_TESTS
