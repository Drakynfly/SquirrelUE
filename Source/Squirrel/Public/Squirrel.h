// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "UObject/Object.h"

#include "Squirrel.generated.h"

/*
 *					WARNING:
 *	READ BEFORE MAKING ANY CHANGES THIS FILE:
 *	This file generates seeded random numbers for
 *	game code. Any changes made here may affect
 *	generation such that existing seeds no longer
 *	function as they previously did. Only make
 *	changes to this file if you are aware of this,
 *	understand what you are doing, or don't care!
 */

DECLARE_LOG_CATEGORY_EXTERN(LogSquirrel, Log, All)

USTRUCT(BlueprintType)
struct SQUIRREL_API FSquirrelState
{
	GENERATED_BODY()

	/** Location in the noise. Use this to "scrub" generation forward and backward. */
	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category = "SquirrelState")
	int32 Position = 0;

#if WITH_EDITOR
	void RandomizeState();
#endif
};

namespace Squirrel
{
	namespace Impl
	{
		// Direct access to calling SquirrelNoise5
		[[nodiscard]] constexpr uint32 SquirrelNoise5(int32& Position, uint32 Seed);
	}

	// Use SquirrelNoise to mangle two values together.
	SQUIRREL_API [[nodiscard]] uint32 HashCombine(int32 A, int32 B);

	uint32 GetGlobalSeed();

	void SetGlobalSeed(uint32 Seed);

	template <
		typename T
		UE_REQUIRES(TIsIntegral<T>::Value)
	>
	[[nodiscard]] constexpr T Next(FSquirrelState& State)
	{
		if constexpr (sizeof(T) >= 4)
		{
			return static_cast<T>(Impl::SquirrelNoise5(State.Position, GetGlobalSeed()));
		}
		else
		{
			return static_cast<T>(Impl::SquirrelNoise5(State.Position, GetGlobalSeed()) % TNumericLimits<T>::Max());
		}
	}

	template <>
	[[nodiscard]] constexpr bool Next(FSquirrelState& State)
	{
		return !!(Impl::SquirrelNoise5(State.Position, GetGlobalSeed()) % 2);
	}

	SQUIRREL_API constexpr int32 NextInt32(FSquirrelState& State, const int32 Max);

	SQUIRREL_API constexpr int32 NextInt32InRange(FSquirrelState& State, const int32 Min, const int32 Max);

	SQUIRREL_API constexpr double NextReal(FSquirrelState& State);

	SQUIRREL_API constexpr double NextRealInRange(FSquirrelState& State, const double Min, const double Max);

	/**
	 * Roll for a deterministic chance of an event occurring.
	 *
	 * @param State Squirrel position
	 * @param Roll The resulting value from the roll. Will always be a value between 0 and 100
	 * @param Chance The percentage chance for the event to occur. Roll must meet or exceed this to succeed.
	 * @param RollModifier A modifier to adjust the likelihood of the occurence. Must be a value between -100 and 100
	 * @return True if the event should occur
	 */
	SQUIRREL_API [[nodiscard]] constexpr bool RollChance(FSquirrelState& State, double& Roll, const double Chance, const double RollModifier);

	/**
	 * Round a float to an int with a chanced result, where the result is determined by the decimal.
	 * Example: Value = 3.25 has a 25% chance to return 4 and a 75% chance to return 3.
	 */
	SQUIRREL_API [[nodiscard]] constexpr int32 RoundWithWeightByFraction(FSquirrelState& State, double Value);
}

/**
 * A noise-based random number generator using SquirrelNoise5
 */
UCLASS(BlueprintType, EditInlineNew, CollapseCategories)
class SQUIRREL_API USquirrel final : public UObject
{
	GENERATED_BODY()

public:
	USquirrel();

	virtual void PostInitProperties() override;

	FSquirrelState& GetState() { return State; }

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
	 * Roll for a deterministic chance of an event occurring.
	 *
	 * @param Roll The resulting value from the roll. Will always be a value between 0 and 100
	 * @param Chance The percentage chance for the event to occur. Roll must meet or exceed this to succeed.
	 * @param RollModifier A modifier to adjust the likelihood of the occurence. Must be a value between -100 and 100
	 * @return True if the event should occur
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


/**
 * Combines the global seed and subsystem state into an easily serialized struct.
 */
USTRUCT(BlueprintType)
struct FSquirrelWorldState
{
	GENERATED_BODY()

	// The world's seed. This is the static value that seeds the game.
	UPROPERTY()
	uint32 GlobalSeed = 0;

	// The position of the Squirrel Subsystem.
	UPROPERTY()
	FSquirrelState RuntimeState;
};


/*
 * This subsystem's primary responsibility to provide seeded positions for new USquirrels generated at runtime.
 */
UCLASS()
class SQUIRREL_API USquirrelSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

	friend USquirrel;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	// Get a new position for a Squirrel that is created during gameplay.
	int32 NewPosition();

public:
	UFUNCTION(BlueprintCallable, Category = "Squirrel")
	int64 GetGlobalSeed() const;

	UFUNCTION(BlueprintCallable, Category = "Squirrel")
	void SetGlobalSeed(int64 NewSeed);

	UFUNCTION(BlueprintCallable, Category = "Squirrel")
	FSquirrelWorldState SaveWorldState() const;

	UFUNCTION(BlueprintCallable, Category = "Squirrel")
	void LoadGameState(FSquirrelWorldState State);

private:
	UPROPERTY(Transient)
	FSquirrelState RuntimePositionsSquirrel;
};