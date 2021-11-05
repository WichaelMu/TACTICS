// Fill out your copyright notice in the Description page of Project Settings.


#include "Block.h"
#include "MouseController.h"
#include "EngineUtils.h"
#include "MW.h"


// Sets default values
ABlock::ABlock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;


	AIAttacked    = 0;
	HumanAttacked = 0;

	SetReplicates(true);
	bAlwaysRelevant = true;
}


// Called when the game starts or when spawned
void ABlock::BeginPlay()
{
	Super::BeginPlay();

	// ...


}


// Predict the future movement of the NearestHeuristic's closest Warrior, limited by Depth.
TArray<ABlock*> ABlock::ComputeTrajectory(ABlock* NearestHeuristic, uint8 Depth)
{
	// Default From and To blocks to be the Nearest Heuristic as a default.
	ABlock* From = NearestHeuristic;
	ABlock* To = NearestHeuristic;

	// Find the nearest Human Warrior with the given heuristic.
	TArray<ABlock*> NeighbouringWarrior = NearestHeuristic->GetNeighbours();
	for (ABlock* Query : NeighbouringWarrior)
	{
		AWarrior* Warrior = Query->Occupant;
		if (Warrior)
		{
			if (Warrior->Affiliation == EAffiliation::HUMAN1 || Warrior->Affiliation == EAffiliation::HUMAN2)
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

	// Find the last orientation of the target Warrior.
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

	// Calculate the future trajectory, based on the last orientation.
	while (Depth--)
	{
		// Continue going in this orientation until the depth limit is reached, or until there is no more block.

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

	// The result is the trajectory based off of the last Warrior's orientation, limited at depth.
	return PossibleTrajectory;
}

void ABlock::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// ...

	DOREPLIFETIME(ABlock, Index);
	DOREPLIFETIME(ABlock, AIAttacked);
	DOREPLIFETIME(ABlock, HumanAttacked);

	DOREPLIFETIME(ABlock, Type);

	DOREPLIFETIME(ABlock, North);
	DOREPLIFETIME(ABlock, South);
	DOREPLIFETIME(ABlock, East);
	DOREPLIFETIME(ABlock, West);

	DOREPLIFETIME(ABlock, NorthEast);
	DOREPLIFETIME(ABlock, NorthWest);
	DOREPLIFETIME(ABlock, SouthEast);
	DOREPLIFETIME(ABlock, SouthWest);

	DOREPLIFETIME(ABlock, Occupant);
}


// Called using blueprint OnMouseClicked.
void ABlock::OnBlockClicked()
{
	for (TActorIterator<AMouseController> it(GetWorld()); it; ++it)
	{
		(*it)->OnBlockClicked(this);
	}
	//AMouseController::BlockClicked(this);
}


// What happens if this block is marked as being selected.
void ABlock::Selected(bool bSelected)
{
	ServerSelected(bSelected);
}

void ABlock::ServerSelected_Implementation(bool bSelected)
{
	// When selected.
	//if (bSelected)
	//{
	//	//SetActorScale3D(FVector(.5f, .5f, .5f));

	//	// Registers which Blocks are traversable.
	//	if (AMouseController::Instance)
	//	{
	//		//AMouseController::Instance->Traversable = GetTraversableBlocks();
	//	}
	//}

	// Show the white tiles?
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
ABlock* ABlock::Get(const uint8& Orientation) const
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


// Get Warrior-traversable blocks and visually marks them for selection.
TArray<ABlock*> ABlock::GetTraversableBlocks()
{
	TArray<ABlock*> Blocks;
	SearchDepthInitialise(Blocks, 2);
	if (AMouseController::CurrentTurn == EAffiliation::HUMAN1 || AMouseController::CurrentTurn == EAffiliation::HUMAN2)
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


// Get this block's neighbours.
TArray<ABlock*> ABlock::GetNeighbours() const
{
	TArray<ABlock*> Neighbours;

	for (int i = 0; i < 8; ++i)
		if (Get(i))
			Neighbours.Add(Get(i));

	return Neighbours;
}


// A depth-limited BFS, whilst considering occupants.
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


// The implementation of BFS at depth, whilst considering occupants.
void ABlock::SearchDepthLogic(TArray<ABlock*>& Blocks, uint8 Depth, TSet<ABlock*>& Visited, TQueue<ABlock*>& Breadth, const bool& bIgnoreOccupants)
{
	if (Depth <= 0) { return; }

	// A queue for the next depth to process.
	// Do not process blocks in this queue, they'll be done in the next depth. (Provided the depth allows for it).
	TQueue<ABlock*> NextDepthQueue;

	// Simple BFS, recall from DSA.
	while (!Breadth.IsEmpty())
	{
		ABlock* FrontBlock = *Breadth.Peek();
		Breadth.Pop();

		// Go FrontBlock's neighbours.
		for (int i = 0; i < 8; ++i)
		{
			ABlock* QueryBlock = FrontBlock->Get(i);

			if (QueryBlock)
			{
				// Skip this iteration if the function asks to ignore occupants.
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
				
				// Don't bother wasting time processing an already-visited block.
				if (!Visited.Contains(QueryBlock))
				{
					Visited.Add(QueryBlock);

					if (!Blocks.Contains(QueryBlock))
					{
						Blocks.Add(QueryBlock);
					}

					NextDepthQueue.Enqueue(QueryBlock);
				}
			}
		}
	}

	// Recursively call this method, passing Blocks and Visited as references to carry on.
	// Pass the NextDepthQueue as blocks that need to be processed at the next depth.
	SearchDepthLogic(Blocks, Depth - 1, Visited, NextDepthQueue, bIgnoreOccupants);
}


/// <summary>Gets any warrior that is in-range of this block.</summary>
/// <param name="RelativeTo">Finds opposing affiliations, relative to this Affiliation.</param>
TArray<AWarrior*> ABlock::SurroundingEnemiesInRange(EAffiliation RelativeTo)
{
	// A warrior is in range if they are at least 3 blocks away. (Movement Range = 2, Attack Range = 1).
	TArray<ABlock*> Range = SearchAtDepth(3);
	TArray<AWarrior*> Enemies;

	// If any block in range has an opponent RelativeTo, add them.
	for (ABlock* W : Range)
	{
		if (W->Occupant)
		{
			if (W->Occupant->Affiliation != RelativeTo)
			{
				// UE_LOG(LogTemp, Warning, TEXT("Block: %s has %s"), *W->GetName(), *W->Occupant->GetName());
				Enemies.Add(W->Occupant);
			}
		}
	}

	return Enemies;
}


// Is this block directly next to the opposition RelativeTo.
bool ABlock::IsNextToAffiliation(const EAffiliation& RelativeTo)
{
	// For every neighbour.
	for (int i = 0; i < 8; ++i)
	{
		ABlock* NeighbouringBlock = Get(i);
		if (NeighbouringBlock)
		{
			AWarrior* Warrior = NeighbouringBlock->Occupant;
			
			// If the neighbour is occupied with a Warrior and is a member of the opposition.
			if (Warrior)
			{
				if (Warrior->Affiliation == RelativeTo)
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
	ServerDeductAttacks(DeductingAffiliation);
}


void ABlock::ServerDeductAttacks_Implementation(EAffiliation DeductingAffiliation)
{
	// Attacking range is 3. (Movement Range = 2, Attacking Range = 1).
	// Do not ignore occupants, we need to deduct from blocks regardless of occupancy.
	TArray<ABlock*> Depth = SearchAtDepth(3, false);

	// Deduct for humans.
	if (DeductingAffiliation == EAffiliation::HUMAN1 || DeductingAffiliation == EAffiliation::HUMAN2)
	{
		// Deduct from this block.
		HumanAttacked--;

		// Deduct from blocks around Depth.
		for (ABlock* Block : Depth)
		{
			// Clamp the number of attacks to a minimum of zero.
			Block->HumanAttacked = FMath::Max<int>(0, Block->HumanAttacked - 1);
		}
	}
	else // Deduct for AI.
	{
		// Deduct from this block.
		AIAttacked--;

		// Deduct from blocks around Depth.
		for (ABlock* Block : Depth)
		{
			// Clamp the number of attacks to a minimum of zero.
			Block->AIAttacked = FMath::Max<int>(0, Block->AIAttacked - 1);
		}
	}
}

/// <summary>Append the attacking heuristic.</summary>
/// <param name="DeductingAffiliation">The Affiliation to append to.</param>
void ABlock::AppendAttacks(EAffiliation AppendingAffiliation)
{
	ServerAppendAttacks(AppendingAffiliation);
}

void ABlock::ServerAppendAttacks_Implementation(EAffiliation AppendingAffiliation)
{
	// Attacking range is 3. (Movement Range = 2, Attacking Range = 1).
	// Do not ignore occupants, we need to deduct from blocks regardless of occupancy.
	TArray<ABlock*> Depth = SearchAtDepth(3, false);

	// Append for humans.
	if (AppendingAffiliation == EAffiliation::HUMAN1 || AppendingAffiliation == EAffiliation::HUMAN2)
	{
		// Append to blocks around Depth.
		for (ABlock* Block : Depth)
		{
			Block->HumanAttacked++;
		}
	}
	else // Append for AI.
	{
		// Append to blocks around Depth.
		for (ABlock* Block : Depth)
		{
			Block->AIAttacked++;
		}
	}
}

