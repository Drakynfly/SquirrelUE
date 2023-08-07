﻿// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Squirrel.h"
#include "SquirrelNoise5.hpp"

DEFINE_LOG_CATEGORY(LogSquirrel)

namespace Squirrel
{
	// The master seed used to set the game's world to a single seed that controls all others.
	static uint32 GWorldSeed = 0;

	int32 NextInt32(FSquirrelState& State)
	{
		return static_cast<int32>(SquirrelNoise5(State.Position++, GWorldSeed));
	}

	int32 NextInt32(FSquirrelState& State, const int32 Max)
	{
		return Max > 0 ? FMath::Min(FMath::TruncToInt(NextReal(State) * static_cast<float>(Max)), Max - 1) : 0;
	}

	int32 NextInt32InRange(FSquirrelState& State, const int32 Min, const int32 Max)
    {
		const int32 Range = (Max - Min) + 1;
		return Min + NextInt32(State, Range);
    }

	bool NextBool(FSquirrelState& State)
	{
		return SquirrelNoise5(State.Position++, GWorldSeed) % 2;
	}

	double NextReal(FSquirrelState& State)
	{
		return Get1dNoiseZeroToOne(State.Position++, GWorldSeed);
	}

	double NextRealInRange(FSquirrelState& State, const double Min, const double Max)
	{
		return Min + (Max - Min) * NextReal(State);
	}

	bool RollChance(FSquirrelState& State, double& Roll, const double Chance, const double RollModifier)
	{
		if (ensure((Chance >= 0.0) && (Chance <= 100.0)) ||
			ensure((RollModifier >= -100.0) && (RollModifier <= 100.0)))
		{
			UE_LOG(LogSquirrel, Warning, TEXT("Bad inputs passed to USquirrel::RollChance - Chance: %f, Modifier: %f"), Chance, RollModifier);
		}

		Roll = NextRealInRange(State, 0.0, 100.0 - RollModifier) + RollModifier;
		return Roll >= Chance;
	}

	int32 RoundWithWeightByFraction(FSquirrelState& State, const double Value)
	{
		double const Whole = FMath::Floor(Value);
		double const Remainder = Value - Whole;

		// If the Remainder equals to 0.f then always return the whole number.
		// @todo probably we want to remove this, right?? it will mess with determinism if this function doesnt *always* increment itself, no?
		if (Remainder <= 0.0)
		{
			return Whole;
		}

		// Otherwise return the whole number plus the random weighted bool.
		return Whole + (Remainder >= NextReal(State));
	}
}

#define Squirrel_NewPositionMin TNumericLimits<int32>::Min()
#define Squirrel_NewPositionMax TNumericLimits<int32>::Max()

#define LOCTEXT_NAMESPACE "Squirrel"

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
			// At runtime, new Squirrels should be given a random (but still seeded) starting position
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
				// It is allowable and expected to get a non-seeded random value in the editor
				State.Position = FMath::RandRange(Squirrel_NewPositionMin, Squirrel_NewPositionMax);
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
	return Squirrel::NextBool(State);
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

int32 USquirrelSubsystem::NewPosition()
{
	return Squirrel::NextInt32InRange(RuntimePositionsSquirrel, Squirrel_NewPositionMin, Squirrel_NewPositionMax);
}

uint32 USquirrelSubsystem::GetGlobalSeed()
{
	return Squirrel::GWorldSeed;
}

void USquirrelSubsystem::SetGlobalSeed(const uint32 Seed)
{
	Squirrel::GWorldSeed = Seed;
}

#undef LOCTEXT_NAMESPACE