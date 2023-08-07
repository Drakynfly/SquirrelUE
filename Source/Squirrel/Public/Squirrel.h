// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "UObject/Object.h"

#include "Squirrel.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSquirrel, Log, All)

USTRUCT(BlueprintType)
struct SQUIRREL_API FSquirrelState
{
	GENERATED_BODY()

	/** Location along sequence. Use this to "scrub" generation forward and backward. */
	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category = "SquirrelState")
	int32 Position = 0;
};

namespace Squirrel
{
	int32 NextInt32(FSquirrelState& State);

	int32 NextInt32(FSquirrelState& State, const int32 Max);

	int32 NextInt32InRange(FSquirrelState& State, const int32 Min, const int32 Max);

	bool NextBool(FSquirrelState& State);

	double NextReal(FSquirrelState& State);

	double NextRealInRange(FSquirrelState& State, const double Min, const double Max);

	/**
	 * Roll for a deterministic, but random chance of an event occurring.
	 *
	 * @param State Squirrel position & seed
	 * @param Roll The resulting value from the roll. Will always be a value between 0 and 100
	 * @param Chance The percentage chance for the event to occur. Roll must meet or exceed this to succeed.
	 * @param RollModifier A modifier to adjust the likelihood of the occurence. Must be a value between -100 and 100
	 */
	bool RollChance(FSquirrelState& State, double& Roll, const double Chance, const double RollModifier);

	/**
	 * Round a float to an int with a chanced result, where the result is determined by the decimal.
	 * Example: Value = 3.25 has a 25% chance to return 4 and a 75% chance to return 3.
	 */
	int32 RoundWithWeightByFraction(FSquirrelState& State, double Value);
}

/**
 * A noise-based random number generator using SquirrelNoise5
 */
UCLASS(BlueprintType, EditInlineNew)
class SQUIRREL_API USquirrel final : public UObject
{
	GENERATED_BODY()

public:
	USquirrel();

	virtual void PostInitProperties() override;

	UFUNCTION(BlueprintCallable, Category = "Squirrel")
	void Jump(const int32 NewPosition);

	UFUNCTION(BlueprintCallable, Category = "Squirrel")
	int32 GetPosition() const;

	UFUNCTION(BlueprintCallable, Category = "Squirrel")
	int32 NextInt32(const int32 Max);

	UFUNCTION(BlueprintCallable, Category = "Squirrel")
	int32 NextInt32InRange(const int32 Min, const int32 Max);

	UFUNCTION(BlueprintCallable, Category = "Squirrel")
	bool NextBool();

	UFUNCTION(BlueprintCallable, Category = "Squirrel")
	double NextReal();

	UFUNCTION(BlueprintCallable, Category = "Squirrel")
	double NextRealInRange(const double Min, const double Max);

	/**
	 * Roll for a deterministic, but random chance of an event occurring.
	 *
	 * @param Roll The resulting value from the roll. Will always be a value between 0 and 100
	 * @param Chance The percentage chance for the event to occur. Roll must meet or exceed this to succeed.
	 * @param RollModifier A modifier to adjust the likelihood of the occurence. Must be a value between -100 and 100
	 */
	UFUNCTION(BlueprintCallable, Category = "Squirrel")
	bool RollChance(double& Roll, const double Chance, const double RollModifier);

	/**
	 * Round a float to an int with a chanced result, where the result is determined by the decimal.
	 * Example: Value = 3.25 has a 25% chance to return 4 and a 75% chance to return 3.
	 */
	UFUNCTION(BlueprintCallable, Category = "Squirrel")
	int32 RoundWithWeightByFraction(double Value);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Squirrel")
	FSquirrelState State;
};


UCLASS()
class SQUIRREL_API USquirrelSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override {}
	virtual void Deinitialize() override {}

	// Get a new position for a Squirrel that is created during gameplay.
	int32 NewPosition();

	static uint32 GetGlobalSeed();

	static void SetGlobalSeed(uint32 Seed);

private:
	UPROPERTY(Transient)
	FSquirrelState RuntimePositionsSquirrel;
};