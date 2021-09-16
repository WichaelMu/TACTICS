// Fill out your copyright notice in the Description page of Project Settings.


#include "MapMaker.h"
#include "Engine/World.h"
#include "Warrior.h"
#include "MW.h"

// Sets default values for this component's properties
UMapMaker::UMapMaker()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...

	Instance = this;
}


// Called when the game starts
void UMapMaker::BeginPlay()
{
	Super::BeginPlay();

	// ...

	// Something is very wrong if there is no block.
	// If there is no block, it may be resetting the Block after compiling in UE4.
	// This bug was observed early in the development of Poly, but has been solved
	// without a definite conclusion. It may happen again. DO NOT REMOVE.
	if (!Block || !Grass || !Stone || !Mountain)
	{
		UE_LOG(LogTemp, Error, TEXT("NO BLOCK"));
		return;
	}

	PlaceBlocks();
	ConnectBlocks();
	SpawnWarriors();

}

void UMapMaker::GenerateLargestConcentrationOfHumans()
{
	TArray<ABlock*> CopyOfMap = Instance->Map;

	ABlock* MaxHuman = CopyOfMap[MapMidPoint()];
	int LargestHuman = 0;

	for (int i = 0; i < CopyOfMap.Num(); ++i)
	{
		bool bAIMax = LargestHuman >= Instance->Map.Num() / 2 - 1;

		if (bAIMax)
		{
			break;
		}
		else
		{
			if (CopyOfMap[i]->HumanAttacked > LargestHuman)
			{
				LargestHuman = CopyOfMap[i]->AIAttacked;
				MaxHuman = CopyOfMap[i];
			}
		}

	}

	HumanConcentration = MaxHuman;
}

void UMapMaker::GenerateLargestConcentrationOfAI()
{
	TArray<ABlock*> CopyOfMap = Instance->Map;

	ABlock* MaxAI = CopyOfMap[MapMidPoint()];
	int LargestAI = 0;

	for (int i = 0; i < CopyOfMap.Num(); ++i)
	{
		bool bAIMax = LargestAI >= Instance->NumberOfWarriors / 2 - 1;

		if (bAIMax)
		{
			break;
		}
		else
		{
			if (CopyOfMap[i]->AIAttacked > LargestAI)
			{
				LargestAI = CopyOfMap[i]->AIAttacked;
				MaxAI = CopyOfMap[i];
			}
		}

	}

	AIConcentration = MaxAI;
}

// A random block in Map.
ABlock* UMapMaker::RandomBlock()
{
	int RandomBlockIndex = FMath::RandRange(0, Map.Num() - 1);

	return Map[RandomBlockIndex];
}

int UMapMaker::MapMidPoint()
{
	int IX = Instance->XMap / 2;
	int IY = Instance->YMap / 2;

	return IY * Instance->XMap + IX;
}

void UMapMaker::PlaceBlocks()
{
	float Offset = FMath::RandRange(-10000.f, 10000.f);

	TArray<float> FalloffMap;
	if (bUseFalloffMap)
	{
		FalloffMap = GenerateFalloffMap();
	}

	TArray<float> Continents;
	if (bGenerateContinents)
	{
		Continents = GenerateContinents();
	}

	// Spawn blocks.
	for (int y = 0; y < YMap; ++y)
	{
		for (int x = 0; x < XMap; ++x)
		{
			int Position = y * XMap + x;

			float Perlin = FMath::PerlinNoise2D(FVector2D(x + Offset, y + Offset) * Roughness);

			// Set Perlin to be between 0 and 1.
			Perlin = (Perlin + 1) / 2;

			if (bUseFalloffMap)
			{
				Perlin -= FalloffMap[Position];
			}

			if (bGenerateContinents)
			{
				Perlin -= Continents[Position];
			}

			if (Perlin < TypeLimits.X)
			{
				SpawnBlock(Water, x, y, EType::WATER);
				continue;
			}
			else if (Perlin < TypeLimits.Y)
			{
				SpawnBlock(Grass, x, y, EType::GRASS);
				continue;
			}
			else if (Perlin < TypeLimits.Z)
			{
				SpawnBlock(Stone, x, y, EType::STONE);
				continue;
			}

			SpawnBlock(Mountain, x, y, EType::MOUNTAIN);
		}
	}
}

ABlock* UMapMaker::SpawnBlock(UClass* Class, const int& X, const int& Y, EType TerrainType)
{
	ABlock* NewBlock = GetWorld()->SpawnActor<ABlock>(Class, FVector(X * 100, Y * 100, 0), FRotator::ZeroRotator);
	Map.Add(NewBlock);
	NewBlock->Index = Y * XMap + X;
	NewBlock->Type = TerrainType;

	return NewBlock;
}

void UMapMaker::ConnectBlocks()
{
	// Connect neighbours.
	for (int y = 0; y < YMap; ++y)
	{
		for (int x = 0; x < XMap; ++x)
		{
			uint16 Index = y * XMap + x;

			if (Map[Index]->Type == EType::MOUNTAIN || Map[Index]->Type == EType::WATER)
			{
				continue;
			}

			//	North
			if (IsIndexInMapRange(x	   , y + 1, Index)) { Map[Index]->North		= Map[ Index       + YMap];	}
			//	North-East
			if (IsIndexInMapRange(x - 1, y + 1, Index)) { Map[Index]->NorthEast	= Map[(Index - 1) + YMap];	}
			//	North-West
			if (IsIndexInMapRange(x + 1, y + 1, Index)) { Map[Index]->NorthWest	= Map[(Index + 1) + YMap];	}

			//	South
			if (IsIndexInMapRange(x    , y - 1, Index)) { Map[Index]->South		= Map[ Index       - YMap];	}
			//	South-East
			if (IsIndexInMapRange(x - 1, y - 1, Index)) { Map[Index]->SouthEast	= Map[(Index - 1) - YMap];	}
			//	South-West
			if (IsIndexInMapRange(x + 1, y - 1, Index)) { Map[Index]->SouthWest	= Map[(Index + 1) - YMap];	}

			//	East
			if (IsIndexInMapRange(x - 1, y    , Index)) { Map[Index]->East		= Map[ Index - 1];		}

			//	West
			if (IsIndexInMapRange(x + 1, y    , Index)) { Map[Index]->West		= Map[ Index + 1];		}
		}
	}
}

// Spawns 1 warrior for each affiliation for NumberOfWarrior times.
void UMapMaker::SpawnWarriors()
{
	// Spawn warriors.
	for (int i = 0; i < NumberOfWarriors; ++i)
	{
		ABlock* RandomWarriorSpawnPoint = RandomBlock();

		while (!UMW::IsBlockTraversable(RandomWarriorSpawnPoint))
		{
			RandomWarriorSpawnPoint = RandomBlock();
		}

		// Spawn 1 warrior, for each team, given NumberOfWarriors.

		AWarrior* SpawnedWarrior1 = GetWorld()->SpawnActor<AWarrior>(Warrior, FVector::ZeroVector, FRotator::ZeroRotator);
		SpawnedWarrior1->OnSpawn(RandomWarriorSpawnPoint, EAffiliation::HUMAN);
		// Defined in blueprint.
		SpawnedWarrior1->AssignAffiliationColours();
		AllWarriors.Add(SpawnedWarrior1);

		while (!UMW::IsBlockTraversable(RandomWarriorSpawnPoint))
		{
			RandomWarriorSpawnPoint = RandomBlock();
		}

		AWarrior* SpawnedWarrior2 = GetWorld()->SpawnActor<AWarrior>(Warrior, FVector::ZeroVector, FRotator::ZeroRotator);
		SpawnedWarrior2->OnSpawn(RandomWarriorSpawnPoint, EAffiliation::AI);
		// Defined in blueprint.
		SpawnedWarrior2->AssignAffiliationColours();
		AllWarriors.Add(SpawnedWarrior2);
	}
}

TArray<float> UMapMaker::GenerateFalloffMap()
{
	TArray<float> FalloffValues;
	FalloffValues.Init(0, XMap * YMap);

	for (int y = 0; y < YMap; ++y)
	{
		for (int x = 0; x < XMap; ++x)
		{
			int Position = y * XMap + x;

			float XFalloff = x / (float)XMap * 2 - 1;
			float YFalloff = y / (float)YMap * 2 - 1;

			float ClosestToEdge = FMath::Max(FMath::Abs(XFalloff), FMath::Abs(YFalloff));

			float ClosestRaisedToStrength = FMath::Pow(ClosestToEdge, CurveStrength);
			float ClosestBias = FMath::Pow(FalloffBias - FalloffBias * ClosestToEdge, CurveStrength);
			float Function = ClosestRaisedToStrength / (ClosestRaisedToStrength + ClosestBias);

			FalloffValues[Position] = Function;
		}
	}

	return FalloffValues;
}

TArray<float> UMapMaker::GenerateContinents()
{
	TArray<float> Continents;
	Continents.Init(0, XMap * YMap);

	float Offset = FMath::RandRange(-10000.f, 10000.f);

	for (int y = 0; y < YMap; ++y)
	{
		for (int x = 0; x < XMap; ++x)
		{
			int Position = y * XMap + x;

			float Perlin = FMath::PerlinNoise2D(FVector2D(x + Offset, y + Offset) * SplitRoughness);

			Perlin = (Perlin + 1) / 2;

			if (Perlin < SplitLimit)
			{
				Continents[Position] = FMath::Clamp<float>(Perlin + SplitStrength, 0, 1);
			}
		}
	}

	return Continents;
}

// If X and Y are in the Map, given the relative Index.
bool UMapMaker::IsIndexInMapRange(const uint16& X, const uint16& Y, const uint16& Index) const
{
	int Position = Y * XMap + X;

	// If the Position is within Map.
	if (!Map.IsValidIndex(Position))
	{
		return false;
	}

	// If you're on the X border.
	if (Index % XMap == XMap - 1)
	{
		// You can't go beyond the XMap or YMap.
		return !(X + 1 > XMap || Y + 1 > YMap);
	}

	return true;
}

