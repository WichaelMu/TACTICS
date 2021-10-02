// Fill out your copyright notice in the Description page of Project Settings.


#include "PoissonDisc.h"
#include "Block.h"
#include "MapMaker.h"
#include "MW.h"


TSet<int32> UPoissonDisc::Occupied;


// Wipe the occupied TSet.
void UPoissonDisc::Flush()
{
	UPoissonDisc::Occupied.Empty();
}


// Try to find a block that is at least MinimumDistance away from another in MapMaker until IterationsBeforeRejection is reached.
ABlock* UPoissonDisc::Sample(UMapMaker* MapMaker, const uint16& IterationsBeforeRejection, const uint8& MinimumDistance)
{
	// Iterate and try to find a valid Poisson'd block until rejection.
	for (uint16 i = 0; i < IterationsBeforeRejection; ++i)
	{
		// Grab a random block.
		ABlock* PoissonBlock = MapMaker->RandomBlock();
		
		// Don't process this block if it is not traversable.
		if (!UMW::IsBlockTraversable(PoissonBlock))
		{
			continue;
		}

		// Don't process this block if it is taken by another MinimumDistance away.
		if (UPoissonDisc::Occupied.Contains(PoissonBlock->Index))
		{
			continue;
		}

		// Register this block as taken and anything within MinimumDistance from it.
		TArray<ABlock*> SurroundingBlocks = PoissonBlock->SearchAtDepth(MinimumDistance);
		for (ABlock* Surrounding : SurroundingBlocks)
		{
			UPoissonDisc::Occupied.Add(Surrounding->Index);
		}

		// An isolated block at least MinimumDistance away from another.
		return PoissonBlock;
	}

	// Poisson Disc Sampling failed due to rejection, small map, or large MinimumDistance.
	UMW::Log("No Poisson Sampled Block found within " + FString::SanitizeFloat(MinimumDistance) + ". The size of the map may be too small.");
	return nullptr;
}

void UPoissonDisc::Print()
{
	UMW::Log("Length of TSet Occipued is: " + FString::SanitizeFloat(Occupied.Num()));
}

