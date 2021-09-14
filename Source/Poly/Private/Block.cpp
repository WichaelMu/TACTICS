// Fill out your copyright notice in the Description page of Project Settings.


#include "Block.h"
#include "MouseController.h"

// Sets default values
ABlock::ABlock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;


	AIAttacked = 0;
	HumanAttacked = 0;
}

// Called when the game starts or when spawned
void ABlock::BeginPlay()
{
	Super::BeginPlay();
}

// Called using blueprint OnMouseClicked.
void ABlock::OnBlockClicked()
{
	AMouseController::BlockClicked(this);
}

// What happens if this block is marked as being selected.
void ABlock::Selected(bool bSelected)
{
	// When selected.
	if (bSelected)
	{
		SetActorScale3D(FVector(.5f, .5f, .5f));
		AMouseController::Instance->Traversable = GetTraversableBlocks();
	}
	else // When deselected / not selected.
	{
		SetActorScale3D(FVector(1, 1, 1));
	}
}

int ABlock::CompareTo(ABlock* Comparison)
{
	int Compare;
	if (F > Comparison->F)
	{
		Compare = 1;
	}
	else if (F < Comparison->F)
	{
		Compare = -1;
	}
	else
	{
		Compare = 0;
	}

	if (Compare == 0)
	{
		if (H > Comparison->H)
		{
			Compare = 1;
		}
		else if (H < Comparison->H)
		{
			Compare = -1;
		}
		else
		{
			Compare = 0;
		}
	}

	return Compare;
}

// Get neighbour at a clockwise Orientation.
ABlock* ABlock::Get(uint8 Orientation) const
{
	switch (Orientation)
	{
	case 0: return North;
	case 1: return NorthEast;
	case 2: return East;
	case 3: return SouthEast;
	case 4: return South;
	case 5: return SouthWest;
	case 6: return West;
	case 7: return NorthWest;
		
	default:
		UE_LOG(LogTemp, Error, TEXT("Could not convert Index to Orientation.\nReturning North."))
		return North;
	}
}

TArray<ABlock*> ABlock::GetTraversableBlocks()
{
	TArray<ABlock*> Blocks;
	SearchDepthInitialise(Blocks, 2);
	if (AMouseController::CurrentTurn == EAffiliation::HUMAN)
	{
		for (ABlock* Surround : Blocks)
		{
			if (Surround != this)
			{
				Surround->SetActorScale3D(FVector(.5f, .5f, .5f));
			}
		}
	}

	return Blocks;
}

TArray<ABlock*> ABlock::SearchAtDepth(uint8 Range, const bool& bIgnoreOccupants)
{
	TArray<ABlock*> Result;

	SearchDepthInitialise(Result, Range, bIgnoreOccupants);

	return Result;
}

// Searches for blocks at Depth.
void ABlock::SearchDepthInitialise(TArray<ABlock*>& Blocks, uint8 Depth, const bool& bIgnoreOccupants)
{
	TSet<ABlock*> Visited;
	TQueue<ABlock*> Breadth;
	Breadth.Enqueue(this);

	SearchDepthLogic(Blocks, Depth, Visited, Breadth, bIgnoreOccupants);
}

void ABlock::SearchDepthLogic(TArray<ABlock*>& Blocks, uint8 Depth, TSet<ABlock*>& Visited, TQueue<ABlock*>& Breadth, const bool& bIgnoreOccupants)
{
	if (Depth <= 0) { return; }

	TQueue<ABlock*> TempQueue;

	while (!Breadth.IsEmpty())
	{
		ABlock* FrontBlock = *Breadth.Peek();
		Breadth.Pop();

		for (int i = 0; i < 8; ++i)
		{
			ABlock* QueryBlock = FrontBlock->Get(i);

			if (QueryBlock)
			{
				if (bIgnoreOccupants)
				{
					if (QueryBlock->Occupant)
					{
						continue;
					}
				}

				// @TODO: If QueryBlock is a mountain/untraversable, continue.
				if (QueryBlock->Type == EType::MOUNTAIN || QueryBlock->Type == EType::WATER)
				{
					continue;
				}

				if (!Visited.Contains(QueryBlock))
				{
					Visited.Add(QueryBlock);

					if (!Blocks.Contains(QueryBlock))
					{
						Blocks.Add(QueryBlock);
					}

					TempQueue.Enqueue(QueryBlock);
				}
			}
		}
	}

	SearchDepthLogic(Blocks, Depth - 1, Visited, TempQueue, bIgnoreOccupants);
}

// Gets any warrior that is in-range of this block, as well as in the future.
TArray<AWarrior*> ABlock::SurroundingEnemiesInRange(EAffiliation RelativeTo)
{
	TArray<ABlock*> Range = SearchAtDepth(3);
	TArray<AWarrior*> Enemies;

	for (ABlock* W : Range)
	{
		if (W->Occupant)
		{
			if (W->Occupant->Affiliation != EAffiliation::AI)
			{
				UE_LOG(LogTemp, Warning, TEXT("Block: %s has %s"), *W->GetName(), *W->Occupant->GetName());
				Enemies.Add(W->Occupant);
			}
		}
	}

	return Enemies;
}

ABlock* ABlock::GetClosestBlockToAHuman(TArray<ABlock*> RangeOfHumans)
{
	float MinDistance = INT_MAX;
	ABlock* Furthest = this;

	for (ABlock* Query : RangeOfHumans)
	{
		if (Query->IsNextToHuman())
		{
			return Query;
		}

		float ReferenceDistance = FVector::DistSquared(GetWarriorPosition(), Query->GetWarriorPosition());
		if (ReferenceDistance < MinDistance)
		{
			MinDistance = ReferenceDistance;
			Furthest = Query;
		}
	}

	return Furthest;
}

bool ABlock::IsNextToHuman()
{
	for (int i = 0; i < 8; ++i)
	{
		AWarrior* Warrior = Get(i)->Occupant;
		if (Warrior)
		{
			if (Warrior->Affiliation == EAffiliation::HUMAN)
			{
				return true;
			}
		}
	}

	return false;
}

void ABlock::DeductAttacks(EAffiliation DeductingAffiliation)
{
	TArray<ABlock*> Depth = SearchAtDepth(3, false);

	if (DeductingAffiliation == EAffiliation::HUMAN)
	{
		HumanAttacked--;

		for (ABlock* Block : Depth)
		{
			Block->HumanAttacked = FMath::Max<int>(0, Block->HumanAttacked - 1);
		}
	}
	else
	{
		AIAttacked--;

		for (ABlock* Block : Depth)
		{
			Block->AIAttacked = FMath::Max<int>(0, Block->AIAttacked - 1);
		}
	}
}

void ABlock::AppendAttacks(EAffiliation DeductingAffiliation)
{
	TArray<ABlock*> Depth = SearchAtDepth(3, false);

	if (DeductingAffiliation == EAffiliation::HUMAN)
	{
		HumanAttacked++;

		for (ABlock* Block : Depth)
		{
			Block->HumanAttacked++;
		}
	}
	else
	{
		AIAttacked--;

		for (ABlock* Block : Depth)
		{
			Block->AIAttacked++;
		}
	}
}

