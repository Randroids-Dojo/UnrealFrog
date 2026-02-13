// Copyright UnrealFrog Team. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Core/FrogPlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"

#if WITH_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Test: All 6 input actions are created and non-null
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInputSystem_ActionsCreated,
	"UnrealFrog.Input.ActionsCreated",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FInputSystem_ActionsCreated::RunTest(const FString& Parameters)
{
	AFrogPlayerController* Controller = NewObject<AFrogPlayerController>();

	TestNotNull(TEXT("IA_HopUp is created"), Controller->IA_HopUp.Get());
	TestNotNull(TEXT("IA_HopDown is created"), Controller->IA_HopDown.Get());
	TestNotNull(TEXT("IA_HopLeft is created"), Controller->IA_HopLeft.Get());
	TestNotNull(TEXT("IA_HopRight is created"), Controller->IA_HopRight.Get());
	TestNotNull(TEXT("IA_Start is created"), Controller->IA_Start.Get());
	TestNotNull(TEXT("IA_Pause is created"), Controller->IA_Pause.Get());

	return true;
}

// ---------------------------------------------------------------------------
// Test: Mapping context is created and non-null
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInputSystem_MappingContextCreated,
	"UnrealFrog.Input.MappingContextCreated",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FInputSystem_MappingContextCreated::RunTest(const FString& Parameters)
{
	AFrogPlayerController* Controller = NewObject<AFrogPlayerController>();

	TestNotNull(TEXT("IMC_Frogger mapping context is created"), Controller->IMC_Frogger.Get());

	return true;
}

// ---------------------------------------------------------------------------
// Test: Input actions have correct value types (Boolean for discrete input)
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInputSystem_ActionValueTypes,
	"UnrealFrog.Input.ActionValueTypes",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FInputSystem_ActionValueTypes::RunTest(const FString& Parameters)
{
	AFrogPlayerController* Controller = NewObject<AFrogPlayerController>();

	TestEqual(TEXT("HopUp is Boolean"), Controller->IA_HopUp->ValueType, EInputActionValueType::Boolean);
	TestEqual(TEXT("HopDown is Boolean"), Controller->IA_HopDown->ValueType, EInputActionValueType::Boolean);
	TestEqual(TEXT("HopLeft is Boolean"), Controller->IA_HopLeft->ValueType, EInputActionValueType::Boolean);
	TestEqual(TEXT("HopRight is Boolean"), Controller->IA_HopRight->ValueType, EInputActionValueType::Boolean);
	TestEqual(TEXT("Start is Boolean"), Controller->IA_Start->ValueType, EInputActionValueType::Boolean);
	TestEqual(TEXT("Pause is Boolean"), Controller->IA_Pause->ValueType, EInputActionValueType::Boolean);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
