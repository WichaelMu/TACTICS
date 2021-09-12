// Fill out your copyright notice in the Description page of Project Settings.


#include "MW.h"
#include "Block.h"
#include "MapMaker.h"
#include "Moves.h"
#include "MouseController.h"
#include "Warrior.h"


bool UMW::Pathfind(ABlock* Origin, ABlock* Destination, TArray<ABlock*>& Path)
{
	TArray<ABlock*> Open;
	Open.Add(Origin);
	Origin->G = 0;
	Origin->H = FVector::Dist(Origin->GetActorLocation(), Destination->GetActorLocation());
	while (Open.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RUNNING"));

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
			UE_LOG(LogTemp, Warning, TEXT("%d"), Path.Num());
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
			float FUpdatedCost = Current->G + FVector::DistSquared(Current->GetActorLocation(), Current->Get(i)->GetActorLocation());
			if (FUpdatedCost < Current->Get(i)->G)
			{
				Current->Get(i)->G = FUpdatedCost;
				Current->Get(i)->H = FVector::DistSquared(Current->Get(i)->GetActorLocation(), Destination->GetActorLocation());
				Current->Get(i)->Parent = Current;

				if (!Open.Contains(Current->Get(i)))
				{
					UE_LOG(LogTemp, Warning, TEXT("ADDED"));
					Open.Add(Current->Get(i));
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
			if (Current->Get(i))
			{
				float FUpdatedCost = Current->G + FVector::DistSquared(Current->GetActorLocation(), Current->Get(i)->GetActorLocation());
				if (FUpdatedCost < Current->Get(i)->G)
				{
					Current->Get(i)->G = FUpdatedCost;
					Current->Get(i)->H = FVector::DistSquared(Current->Get(i)->GetActorLocation(), Destination->GetActorLocation());
					Current->Get(i)->Parent = Current;

					if (!Open.Contains(Current->Get(i)))
					{
						Open.Add(Current->Get(i));
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Error, TEXT("No Path found!"));

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

void UMW::DetermineMoves()
{
	TArray<TPair<AWarrior*, ABlock*>> Moves;
	TArray<AWarrior*> AIWarriors;
	TArray<AWarrior* > HumanWarriors;

	for (AWarrior* Warrior : UMapMaker::Instance->AllWarriors)
	{
		if (Warrior->Affiliation == EAffiliation::AI)
		{
			AIWarriors.Add(Warrior);
		}
		else
		{
			HumanWarriors.Add(Warrior);
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
