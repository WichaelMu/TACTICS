// Fill out your copyright notice in the Description page of Project Settings.


#include "MW.h"
#include "Block.h"
#include "MapMaker.h"
#include "MouseController.h"
#include "Warrior.h"


/// <summary>A* Pathfind from Origin to Destination.</summary>
/// <param name="Path">Out Path.</param>
/// <returns>True if a path was found from Origin to Destination.</returns>
bool UMW::Pathfind(ABlock* Origin, ABlock* Destination, TArray<ABlock*>& Path)
{
	for (ABlock* Block : UMapMaker::Instance->Map) { Block->G = INT_MAX; }
	
	TArray<ABlock*> Open;
	Open.Add(Origin);
	Origin->G = 0;
	Origin->H = FVector::Dist(Origin->GetActorLocation(), Destination->GetActorLocation());
	while (Open.Num() > 0)
	{
		ABlock* Current = Open[0];

		float LowestF = INT_MAX;
		for (int i = 1; i < Open.Num(); ++i)
		{
			if (Open[i]->F < LowestF)
			{
				LowestF = Open[i]->F;
				Current = Open[i];
			}
		}

		Open.Remove(Current);

		if (Current == Destination)
		{
			ABlock* Traverse = Destination;
			while (Traverse != Origin)
			{
				Path.Add(Traverse);
				Traverse = Traverse->Parent;
			}

			return true;
		}

		for (int i = 0; i < 8; ++i)
		{
			ABlock* Query = Current->Get(i);
			if (Query)
			{
				if (IsBlockTraversable(Query))
				{
					float FUpdatedCost = Current->G + FVector::DistSquared(Current->GetActorLocation(), Query->GetActorLocation());
					if (FUpdatedCost < Query->G)
					{
						Query->G = FUpdatedCost;
						Query->H = FVector::DistSquared(Query->GetActorLocation(), Destination->GetActorLocation());
						Query->Parent = Current;

						if (!Open.Contains(Query))
						{
							Open.Add(Query);
						}
					}
				}
			}
		}
	}

	return false;

}

/// <summary>A* Pathfind from Origin to Destination.</summary>
TArray<ABlock*> UMW::Pathfind(ABlock* Origin, ABlock* Destination)
{
	for (ABlock* Block : UMapMaker::Instance->Map) { Block->G = INT_MAX; }

	TArray<ABlock*> Path;
	TArray<ABlock*> Open;
	Open.Add(Origin);
	Origin->G = 0;
	Origin->H = FVector::DistSquared(Origin->GetActorLocation(), Destination->GetActorLocation());
	while (Open.Num() > 0)
	{
		ABlock* Current = Open[0];

		float LowestF = INT_MAX;
		for (int i = 1; i < Open.Num(); ++i)
		{
			if (Open[i]->F < LowestF)
			{
				LowestF = Open[i]->F;
				Current = Open[i];
			}
		}

		Open.Remove(Current);

		if (Current == Destination)
		{
			ABlock* Traverse = Destination;
			while (Traverse != Origin)
			{
				Path.Add(Traverse);
				Traverse = Traverse->Parent;
			}
			
			return Path;
		}

		for (int i = 0; i < 8; ++i)
		{
			ABlock* Query = Current->Get(i);
			if (Query)
			{
				if (IsBlockTraversable(Query))
				{
					float FUpdatedCost = Current->G + FVector::DistSquared(Current->GetActorLocation(), Query->GetActorLocation());
					if (FUpdatedCost < Query->G)
					{
						Query->G = FUpdatedCost;
						Query->H = FVector::DistSquared(Query->GetActorLocation(), Destination->GetActorLocation());
						Query->Parent = Current;

						if (!Open.Contains(Query))
						{
							Open.Add(Query);
						}
					}
				}
			}
		}
	}
	
	return Path;
}

/// <summary>Whether or not this block is traversable.</summary>
/// <param name="Query">The ABlock in question.</param>
/// <returns>False if this block has an occupant or is a EType::MOUNTAIN or EType::WATER.</returns>
bool UMW::IsBlockTraversable(ABlock* Query)
{
	return !(Query->Occupant || Query->Type == EType::MOUNTAIN || Query->Type == EType::WATER);
}

/// <summary>Compute and determine the AI for Warriors->Affiliation == EAffiliation::AI.</summary>
void UMW::RunAI()
{
	DetermineMoves();
}

/// <summary>UE_LOG's Message to the Output Log with Warning Verbosity.</summary>
/// <param name="Message">The FString to display.</param>
void UMW::Log(FString Message)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
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
		// TODO: If health is low and NOT in attack in 1 ply, revive itself.

		ABlock* CurrentBlock = Evaluating->CurrentBlock;
		//TArray<ABlock*> Traversable = CurrentBlock->GetTraversableBlocks();


		if (Evaluating->Health <= 4)
		{
			Evaluating->Retreat();
		}
		else
		{
			Evaluating->Attack();
		}


		//Moves.Add(TPair<AWarrior*, ABlock*>(Evaluating, Traversable.Last()));
	}
}
