// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "FroggerFunctionalTest.h"
#include "FroggerFunctionalTests.generated.h"

/** Functional test: Hop forward 3 times, verify score = 10+20+30 = 60. */
UCLASS()
class AFT_HopAndScore : public AFroggerFunctionalTest
{
	GENERATED_BODY()
public:
	virtual void StartTest() override;
};

/** Functional test: Spawn car in frog's path, hop into it, verify death + life lost. */
UCLASS()
class AFT_CarCollision : public AFroggerFunctionalTest
{
	GENERATED_BODY()
public:
	virtual void StartTest() override;
};

/** Functional test: Hop onto river row with log, verify mounted (not dead). */
UCLASS()
class AFT_RiverLanding : public AFroggerFunctionalTest
{
	GENERATED_BODY()
public:
	virtual void StartTest() override;
};

/** Functional test: Hop onto empty river row, verify splash death. */
UCLASS()
class AFT_RiverDeath : public AFroggerFunctionalTest
{
	GENERATED_BODY()
public:
	virtual void StartTest() override;
};

/** Functional test: Navigate to home slot, verify slot filled + 200 points. */
UCLASS()
class AFT_HomeSlotFill : public AFroggerFunctionalTest
{
	GENERATED_BODY()
public:
	virtual void StartTest() override;
};
