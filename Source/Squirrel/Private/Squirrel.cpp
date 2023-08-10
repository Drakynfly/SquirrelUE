// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Squirrel.h"
#include "SquirrelNoise5.hpp"

/*
 *					WARNING:
 *	READ BEFORE MAKING ANY CHANGES THIS FILE:
 *	This file generates seeded random numbers for
 *	game code. Any changes made here may affect
 *	generatation such that existing seeds no longer
 *	function as they previously did. Only make
 *	changes to this file, if you are aware of this,
 *	understand what you are doing, or don't care!
 */

DEFINE_LOG_CATEGORY(LogSquirrel)

#define LOCTEXT_NAMESPACE "Squirrel"

namespace Squirrel
{
	// The master seed used to set the game world to a consistant state that can be returned to.
	static uint32 GWorldSeed = 0;

	namespace Impl
	{
		constexpr uint32 SquirrelNoise5(FSquirrelState& State)
		{
			return ::SquirrelNoise5(State.Position++, GWorldSeed);
		}
	}

	namespace Math
	{
		// The purpose of this function is to generate a random value in the full range of its type.
		// RandRange functions like that of FMath can only generate values in the range of ( Min/2+1 -> Max/2 ).
		template <typename T, typename RNG>
		constexpr T MaxRand(RNG Engine)
		{
			union
			{
				T Total;
				uint8 Pieces[sizeof(T)];
			} Value;

			for (size_t i = 0; i < sizeof Value.Pieces; i++)
			{
				Value.Pieces[i] = Engine();
			}

			return Value.Total;
		}

		// constexpr version of FMath::Ceil
		constexpr int64 SqCeil(const double Value)
		{
			const int64 Int = static_cast<int64>(Value);
			return Value > Int ? Int + 1 : Int;
		}

		// constexpr version of FMath::Floor
		constexpr int64 SqFloor(const double Value)
		{
			const int64 Int = static_cast<int64>(Value);
			return Value < Int ? Int - 1 : Int;
		}
	}

	uint32 GetGlobalSeed()
	{
		return GWorldSeed;
	}

	void SetGlobalSeed(const uint32 Seed)
	{
		GWorldSeed = Seed;
	}

	constexpr int32 NextInt32(FSquirrelState& State, const int32 Max)
	{
		return Max > 0 ? FMath::Min(FMath::TruncToInt(NextReal(State) * static_cast<double>(Max)), Max - 1) : 0;
	}

	constexpr int32 NextInt32InRange(FSquirrelState& State, const int32 Min, const int32 Max)
    {
		// @todo Min and Max must only cover *half* the int32 range, or it will cause an overflow in (Max - Min)
		// look into alternate ways to generate random values that don't have this limit

		const int32 Range = (Max - Min) + 1;
		return Min + NextInt32(State, Range);
    }

	constexpr double NextReal(FSquirrelState& State)
	{
		return Get1dNoiseZeroToOne(State.Position++, GWorldSeed);
	}

	constexpr double NextRealInRange(FSquirrelState& State, const double Min, const double Max)
	{
		// @todo Min and Max must only cover *half* the int32 range, or it will cause an overflow in (Max - Min)
		// look into alternate ways to generate random values that don't have this limit

		return Min + (Max - Min) * NextReal(State);
	}

	constexpr bool RollChance(FSquirrelState& State, double& Roll, const double Chance, const double RollModifier)
	{
		if (ensure((Chance >= 0.0) && (Chance <= 100.0)) ||
			ensure((RollModifier >= -100.0) && (RollModifier <= 100.0)))
		{
			// @todo log is broken with constexpr. fix/replace later
			//UE_LOG(LogSquirrel, Warning, TEXT("Bad inputs passed to USquirrel::RollChance - Chance: %f, Modifier: %f"), Chance, RollModifier);
		}

		Roll = NextRealInRange(State, 0.0, 100.0 - RollModifier) + RollModifier;
		return Roll >= Chance;
	}

	constexpr int32 RoundWithWeightByFraction(FSquirrelState& State, const double Value)
	{
		const double Whole = Math::SqFloor(Value);
		const double Remainder = Value - Whole;

		// If the Remainder equals to 0.f then always return the whole number.
		if (Remainder <= 0.0)
		{
			return Whole;
		}

		// Otherwise return the whole number plus the random weighted bool.
		return Whole + (Remainder >= NextReal(State));
	}
}

#if WITH_EDITOR

void FSquirrelState::RandomizeState()
{
	// It is allowable and expected to get a non-seeded random value in the editor, hence using 'Rand' here.
	Position = Squirrel::Math::MaxRand<int32>(FMath::Rand);
}
#endif

USquirrel::USquirrel()
{
}

void USquirrel::PostInitProperties()
{
	Super::PostInitProperties();

	if (!IsTemplate())
	{
		if (const UWorld* World = GEngine->GetWorldFromContextObject(GetTypedOuter<AActor>(), EGetWorldErrorMode::ReturnNull))
		{
			// At runtime, Squirrels should be given random (but still seeded) positions
			if (World->HasBegunPlay())
			{
				if (USquirrelSubsystem* Subsystem = GEngine->GetEngineSubsystem<USquirrelSubsystem>())
				{
					State.Position = Subsystem->NewPosition();
				}
			}
#if WITH_EDITOR
			// In the editor, they should be given a new one in any-case
			else if (GIsEditor)
			{
				State.RandomizeState();
			}
#endif
		}
	}
}

void USquirrel::Jump(const int32 NewPosition)
{
	State.Position = NewPosition;
}

int32 USquirrel::GetPosition() const
{
	return State.Position;
}

int32 USquirrel::NextInt32(const int32 Max)
{
	return Squirrel::NextInt32(State, Max);
}

int32 USquirrel::NextInt32InRange(const int32 Min, const int32 Max)
{
	return Squirrel::NextInt32InRange(State, Min, Max);
}

bool USquirrel::NextBool()
{
	return Squirrel::Next<bool>(State);
}

double USquirrel::NextReal()
{
	return Squirrel::NextReal(State);
}

double USquirrel::NextRealInRange(const double Min, const double Max)
{
	return Squirrel::NextRealInRange(State, Min, Max);
}

bool USquirrel::RollChance(double& Roll, const double Chance, const double RollModifier)
{
	return Squirrel::RollChance(State, Roll, Chance, RollModifier);
}

int32 USquirrel::RoundWithWeightByFraction(const double Value)
{
	return Squirrel::RoundWithWeightByFraction(State, Value);
}

void USquirrelSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if WITH_EDITOR
	if (GIsEditor)
	{
		RuntimePositionsSquirrel.RandomizeState();
	}
#endif
}

void USquirrelSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

int32 USquirrelSubsystem::NewPosition()
{
	return Squirrel::Next<int32>(RuntimePositionsSquirrel);
}

int64 USquirrelSubsystem::GetGlobalSeed() const
{
	return Squirrel::GetGlobalSeed();
}

void USquirrelSubsystem::SetGlobalSeed(const int64 NewSeed)
{
	Squirrel::SetGlobalSeed(NewSeed);
}

FSquirrelWorldState USquirrelSubsystem::SaveWorldState() const
{
	return FSquirrelWorldState{Squirrel::GetGlobalSeed(), RuntimePositionsSquirrel };
}

void USquirrelSubsystem::LoadGameState(const FSquirrelWorldState State)
{
	Squirrel::SetGlobalSeed(State.GlobalSeed);
	RuntimePositionsSquirrel = State.RuntimeState;
}

#undef LOCTEXT_NAMESPACE