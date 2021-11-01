// Fill out your copyright notice in the Description page of Project Settings.


#include "MW.h"
#include "Block.h"
#include "MapMaker.h"
#include "Warrior.h"



/// <summary>Whether or not this block is traversable.</summary>
/// <param name="Query">The ABlock in question.</param>
/// <returns>False if this block has an occupant or is a EType::MOUNTAIN or EType::WATER.</returns>
bool UMW::IsBlockTraversable(ABlock* Query)
{
	return !(Query->Occupant || Query->Type == EType::MOUNTAIN || Query->Type == EType::WATER);
}


// Compute and determine the AI for Warriors->Affiliation == EAffiliation::AI.
void UMW::RunAI()
{
	DetermineMoves();
}


void UMW::DetermineMoves()
{
	TArray<AWarrior*> AIWarriors;

	for (AWarrior* Warrior : UMapMaker::Instance->AllWarriors)
	{
		if (Warrior->Affiliation == EAffiliation::AI)
		{
			AIWarriors.Add(Warrior);
		}
	}

	for (AWarrior* Evaluating : AIWarriors)
	{
		for (ABlock* Block : UMapMaker::Instance->Map)
		{
			Block->G = INT_MAX;
		}

		if (Evaluating->Health <= 4)
		{
			Evaluating->Retreat();
		}
		else
		{
			Evaluating->Attack();
		}
	}
}


/// <summary>UE_LOG's Message to the Output Log with Warning Verbosity.</summary>
/// <param name="Message">The FString to display.</param>
void UMW::Log(FString Message)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
}


/// <summary>UE_LOG's Message to the Output Log with Warning Verbosity.</summary>
/// <param name="Message">The FString to display.</param>
void UMW::LogError(FString Message)
{
	UE_LOG(LogTemp, Error, TEXT("%s"), * Message);
}

