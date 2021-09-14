// Fill out your copyright notice in the Description page of Project Settings.


#include "MW.h"
#include "Block.h"
#include "MapMaker.h"
#include "MouseController.h"
#include "Warrior.h"


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

TArray<ABlock*> UMW::Pathfind(ABlock* Origin, ABlock* Destination)
{
	for (ABlock* Block : UMapMaker::Instance->Map) { Block->G = INT_MAX; }

	TArray<ABlock*> Path;
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

void UMW::RunAI()
{
	DetermineMoves();
}

void UMW::Log(FString Message)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
}

bool UMW::IsBlockTraversable(ABlock* Query)
{
	return !(Query->Occupant || Query->Type == EType::MOUNTAIN || Query->Type == EType::WATER);
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


		/*if (Evaluating->Health <= 4)
		{
			Evaluating->Retreat();
		}*/

		Evaluating->Attack();

		//Moves.Add(TPair<AWarrior*, ABlock*>(Evaluating, Traversable.Last()));
	}
}
