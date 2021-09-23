// Fill out your copyright notice in the Description page of Project Settings.


#include "PoissonDisc.h"
#include "Block.h"
#include "MapMaker.h"
#include "MW.h"


TSet<int32> UPoissonDisc::Occupied;


void UPoissonDisc::Flush()
{
	UPoissonDisc::Occupied.Empty();
}

ABlock* UPoissonDisc::Sample(UMapMaker* MapMaker, const uint16& IterationsBeforeRejection, const uint8& MinimumDistance)
{
	for (uint16 i = 0; i < IterationsBeforeRejection; ++i)
	{
		ABlock* PoissonBlock = MapMaker->RandomBlock();

		if (!UMW::IsBlockTraversable(PoissonBlock))
		{
			continue;
		}

		if (UPoissonDisc::Occupied.Contains(PoissonBlock->Index))
		{
			continue;
		}

		TArray<ABlock*> SurroundingBlocks = PoissonBlock->SearchAtDepth(MinimumDistance);
		for (ABlock* Surrounding : SurroundingBlocks)
		{
			UPoissonDisc::Occupied.Add(Surrounding->Index);
		}

		return PoissonBlock;
	}

	UMW::Log("No Poisson Sampled Block found within " + FString::SanitizeFloat(MinimumDistance) + ". The size of the map may be too small.");
	return nullptr;
}

