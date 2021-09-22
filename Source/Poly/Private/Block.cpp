// Fill out your copyright notice in the Description page of Project Settings.


#include "Block.h"
#include "MouseController.h"
#include "MW.h"

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


	SetTraversableVisibility(false);
}

TArray<ABlock*> ABlock::ComputeTrajectory(ABlock* NearestHeuristic, uint8 Depth)
{
	ABlock* From = NearestHeuristic;
	ABlock* To = NearestHeuristic;

	TArray<ABlock*> NeighbouringWarrior = NearestHeuristic->GetNeighbours();
	for (ABlock* Query : NeighbouringWarrior)
	{
		AWarrior* Warrior = Query->Occupant;
		if (Warrior)
		{
			if (Warrior->Affiliation == EAffiliation::HUMAN)
			{
				From = Warrior->PreviousBlock;
				To = Warrior->CurrentBlock;
				
				break;
			}
		}
	}

	TArray<ABlock*> PossibleTrajectory;
	
	int MinDistance = INT_MAX;
	uint8 Orientation = 0;

	for (uint8 i = 0; i < 8; ++i)
	{
		ABlock* Neighbour = From->Get(i);
		if (Neighbour)
		{
			float Distance = FVector::DistSquared(Neighbour->GetWarriorPosition(), To->GetWarriorPosition());

			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				Orientation = i;
			}
		}
	}

	ABlock* Trajectory = From;

	while (Depth--)
	{
		ABlock* Prediction = Trajectory->Get(Orientation);
		if (Prediction)
		{
			PossibleTrajectory.Add(Prediction);
			Trajectory = Prediction;
		}
		else
		{
			break;
		}
	}

	return PossibleTrajectory;
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
		//SetActorScale3D(FVector(.5f, .5f, .5f));
		AMouseController::Instance->Traversable = GetTraversableBlocks();
	}

	SetTraversableVisibility(bSelected);
}

// Identical to C#'s IComparable<ABlock> CompareTo(ABlock*) override; See /SOUTHLAND.
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

/// <summary>Get Warrior-traversable blocks.</summary>
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
				Surround->SetTraversableVisibility(true);
			}
		}
	}

	return Blocks;
}

/// <summary>Begins a breadth-first search at a depth limiter.</summary>
/// <param name="Depth">The maximum depth to search to.</param>
/// <param name="bIgnoreOccupants">Ignore ABlocks with occupants?</param>
/// <returns>The BFS at a limit of depth, whilst considering occupants.</returns>
TArray<ABlock*> ABlock::SearchAtDepth(uint8 Depth, const bool& bIgnoreOccupants)
{
	TArray<ABlock*> Result;

	SearchDepthInitialise(Result, Depth, bIgnoreOccupants);

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

TArray<ABlock*> ABlock::GetNeighbours() const
{
	TArray<ABlock*> Neighbours;

	for (int i = 0; i < 8; ++i)
		if (Get(i))
			Neighbours.Add(Get(i));

	return Neighbours;
}

/// <summary>Gets any warrior that is in-range of this block.</summary>
/// <param name="RelativeTo">Finds opposing affiliations, relative to this Affiliation.</param>
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

/// <summary>The closest block to a Human out of the Range of Humans.</summary>
/// <param name="RangeOfHumans">The range of blocks to check for the closest Human.</param>
/// <returns>The ABlock to close in on.</returns>
ABlock* ABlock::GetClosestBlockToAHuman(TArray<ABlock*> RangeOfHumans)
{
	float MinDistance = INT_MAX;
	ABlock* Furthest = this;

	for (ABlock* Query : RangeOfHumans)
	{
		if (Query->IsNextToAffiliation(EAffiliation::HUMAN))
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

/// <summary>Is this block directly next to a Human?</summary>
bool ABlock::IsNextToAffiliation(const EAffiliation& Neighbour)
{
	for (int i = 0; i < 8; ++i)
	{
		ABlock* NeighbouringBlock = Get(i);
		if (NeighbouringBlock)
		{
			AWarrior* Warrior = NeighbouringBlock->Occupant;
			if (Warrior)
			{
				if (Warrior->Affiliation == Neighbour)
				{
					return true;
				}
			}
		}
	}

	return false;
}

/// <summary>Deduct the attacking heuristic.</summary>
/// <param name="DeductingAffiliation">The Affiliation to deduct from.</param>
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

/// <summary>Append the attacking heuristic.</summary>
/// <param name="DeductingAffiliation">The Affiliation to append to.</param>
void ABlock::AppendAttacks(EAffiliation DeductingAffiliation)
{
	TArray<ABlock*> Depth = SearchAtDepth(3, false);

	if (DeductingAffiliation == EAffiliation::HUMAN)
	{
		for (ABlock* Block : Depth)
		{
			Block->HumanAttacked++;
		}
	}
	else
	{
		for (ABlock* Block : Depth)
		{
			Block->AIAttacked++;
		}
	}
}

