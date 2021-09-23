// Fill out your copyright notice in the Description page of Project Settings.


#include "MapMaker.h"
#include "Engine/World.h"
#include "Warrior.h"
#include "MW.h"
#include "PoissonDisc.h"

// Sets default values for this component's properties
UMapMaker::UMapMaker()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...

	Instance = this;


	Roughness = .15f;

	FalloffBias = 4.4f;
	CurveStrength = 3.f;

	SplitLimit = .43f;
	SplitDistance = .15f;
	SplitRoughness = .06f;

	WaterLimits = .15f;
	ShallowLimits = .2f;
	SandLimits = .4f;
	GrassLimits = .6f;
	StoneLimits = .65f;
	MountainLimits = 1.f;

	EquatorBias = 10.f;
	EquatorStrength = 15.f;
	EquatorSpread = .45f;
	EquatorRoughness = .15f;

	IterationsBeforeRejection = 30;
	MinimumDistance = 3;
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
	if (!Block || !Grass || !Stone || !Mountain || !Water || !Shallow || !Sand || !Desert)
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

	// Defualt the largest concentration of Humans to the centre of the map.
	if (AWarrior::NumberOfHuman == 0)
	{
		HumanConcentration = MaxHuman;
		return;
	}

	int LargestHuman = 0;

	// For every block in the map.
	for (int i = 0; i < CopyOfMap.Num(); ++i)
	{
		// Limit the search if the largest concetration of Humans is greater than half the map, or if it is greater than half of the initial population of spawned warriors.
		bool bAIMax = LargestHuman >= Instance->Map.Num() / 2 - 1;

		if (bAIMax)
		{
			break;
		}
		else
		{
			// If the search can continue, set the largest concentration.
			if (CopyOfMap[i]->HumanAttacked > LargestHuman)
			{
				LargestHuman = CopyOfMap[i]->AIAttacked;
				MaxHuman = CopyOfMap[i];
			}
		}

	}

	// The block with the largest number of Human control.
	HumanConcentration = MaxHuman;
}

void UMapMaker::GenerateLargestConcentrationOfAI()
{
	TArray<ABlock*> CopyOfMap = Instance->Map;

	ABlock* MaxAI = CopyOfMap[MapMidPoint()];

	// Defualt the largest concentration of AI to the centre of the map.
	if (AWarrior::NumberOfAI == 0)
	{
		AIConcentration = MaxAI;
		return;
	}

	int LargestAI = 0;

	// For every block in the map.
	for (int i = 0; i < CopyOfMap.Num(); ++i)
	{
		// Limit the search if the largest concetration of AI is greater than half the map, or if it is greater than half of the initial population of spawned warriors.
		bool bAIMax = LargestAI >= Instance->NumberOfWarriors / 2 - 1;

		if (bAIMax)
		{
			break;
		}
		else
		{
			// If the search can continue, set the largest concentration.
			if (CopyOfMap[i]->AIAttacked > LargestAI)
			{
				LargestAI = CopyOfMap[i]->AIAttacked;
				MaxAI = CopyOfMap[i];
			}
		}

	}

	// The block with the largest number of AI control.
	AIConcentration = MaxAI;
}

// A random block in Map.
ABlock* UMapMaker::RandomBlock()
{
	int RandomBlockIndex = FMath::RandRange(0, Map.Num() - 1);

	return Map[RandomBlockIndex];
}

// The middle of the map, based on the X-Axis.
int UMapMaker::MapMidPoint()
{
	// The mid point of the map is defined as the (X length + 1) / (Half of X length).
	return (Instance->XMap + 1) * (Instance->XMap / 2);
}

void UMapMaker::PlaceBlocks()
{
	float Offset = FMath::RandRange(-10000.f, 10000.f);
	UMW::Log("Terrain Seed: " + FString::SanitizeFloat(Offset) + " at " + FString::SanitizeFloat(Roughness) + " Roughness.");

	// Should Map Maker generate a Falloff Map?
	TArray<float> FalloffMap;
	if (bUseFalloffMap)
	{
		FalloffMap = GenerateFalloffMap();
	}

	// Should Map Maker generate Continents?
	TArray<float> Continents;
	if (bGenerateContinents)
	{
		Continents = GenerateContinents();
	}

	// Should Map Maker compute the Equator?
	TArray<int> Equator;
	if (bComputeEquatorialEnvironment)
	{
		Equator = ComputeEquator();
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

			// If Continents are requested, subtract Perlin from the Continents mapping.
			if (bGenerateContinents)
			{
				Perlin -= Continents[Position];
			}

			// If a Falloff Map is requested, subtract Perlin from the Falloff Map values.
			if (bUseFalloffMap)
			{
				Perlin -= FalloffMap[Position];
			}

			if (Perlin < WaterLimits)
			{
				SpawnBlock(Water, x, y, EType::WATER);
			}
			else if (Perlin < ShallowLimits)
			{
				SpawnBlock(Shallow, x, y, EType::WATER);
			}
			else if (bComputeEquatorialEnvironment && Perlin < GrassLimits && Equator.Contains(Position))
			{
				SpawnBlock(Desert, x, y, EType::GRASS);
			}
			else if (Perlin < SandLimits)
			{
				SpawnBlock(Sand, x, y, EType::GRASS);
			}
			else if (Perlin < GrassLimits)
			{
				SpawnBlock(Grass, x, y, EType::GRASS);
			}
			else if (Perlin < StoneLimits)
			{
				SpawnBlock(Stone, x, y, EType::STONE);
			}
			else
			{
				SpawnBlock(Mountain, x, y, EType::MOUNTAIN);
			}
		}
	}
}

// Spawns a block into the world and initialises it.
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
	// Do not connect blocks if there warriors are not requested.
	if (NumberOfWarriors == 0) { return; }

	// Connect neighbours.
	for (uint16 y = 0; y < YMap; ++y)
	{
		for (uint16 x = 0; x < XMap; ++x)
		{
			uint16 Index = y * XMap + x;

			// Do not attempt to connect blocks if Index is an EType::Mountain or EType::Water.
			if (Map[Index]->Type == EType::MOUNTAIN || Map[Index]->Type == EType::WATER)
			{
				continue;
			}

			//	North
			if (IsIndexInMapRange(x	   , y + 1, Index)) { Map[Index]->North		= Map[ Index      + YMap];	}
			//	North-East
			if (IsIndexInMapRange(x - 1, y + 1, Index)) { Map[Index]->NorthEast	= Map[(Index - 1) + YMap];	}
			//	North-West
			if (IsIndexInMapRange(x + 1, y + 1, Index)) { Map[Index]->NorthWest	= Map[(Index + 1) + YMap];	}

			//	South
			if (IsIndexInMapRange(x    , y - 1, Index)) { Map[Index]->South		= Map[ Index      - YMap];	}
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
	// Static T template's memory persist. Flush the Poisson Disc occupancy.
	UPoissonDisc::Flush();

	// Spawn warriors.
	for (int i = 0; i < NumberOfWarriors; ++i)
	{
		// Spawn 1 warrior, for each team, given NumberOfWarriors.

		ABlock* PoissonSpawnPoint = GetPoissonOrRandomBlock();

		AWarrior* SpawnedWarrior1 = GetWorld()->SpawnActor<AWarrior>(Warrior, FVector::ZeroVector, FRotator::ZeroRotator);
		SpawnedWarrior1->OnSpawn(PoissonSpawnPoint, EAffiliation::HUMAN);
		AllWarriors.Add(SpawnedWarrior1);

		PoissonSpawnPoint = GetPoissonOrRandomBlock();

		AWarrior* SpawnedWarrior2 = GetWorld()->SpawnActor<AWarrior>(Warrior, FVector::ZeroVector, FRotator::ZeroRotator);
		SpawnedWarrior2->OnSpawn(PoissonSpawnPoint, EAffiliation::AI);
		AllWarriors.Add(SpawnedWarrior2);
	}

	AWarrior::NumberOfAI = NumberOfWarriors;
	AWarrior::NumberOfHuman = NumberOfWarriors;
}

// Gets a block either MinimumDistance apart from another Warrior, or a random block on failure.
ABlock* UMapMaker::GetPoissonOrRandomBlock()
{
	ABlock* PoissonSpawnPoint = nullptr;
	
	if (bGeneratePoissonSpawning)
	{
		PoissonSpawnPoint = UPoissonDisc::Sample(this, IterationsBeforeRejection, MinimumDistance);
	}

	// If Poisson Disc Sampling failed.
	if (!PoissonSpawnPoint)
	{
		// Find a random block.
		do
		{
			PoissonSpawnPoint = RandomBlock();
		} while (!UMW::IsBlockTraversable(PoissonSpawnPoint));
	}

	return PoissonSpawnPoint;
}

// Generates a Falloff Map. (A Map surrounded by Water/Shallow; an island).
TArray<float> UMapMaker::GenerateFalloffMap()
{
	TArray<float> FalloffValues;

	// Initialise to all zeroes.
	FalloffValues.Init(0, XMap * YMap);

	for (int y = 0; y < YMap; ++y)
	{
		for (int x = 0; x < XMap; ++x)
		{
			int Position = y * XMap + x;

			// The ratio distance between this X and Y block from the edge of the intended Map.
			// Clamped to be between -1 and 1.
			float XFalloff = x / (float)XMap * 2 - 1;
			float YFalloff = y / (float)YMap * 2 - 1;

			// Which X or Y Falloff is closest to the edge of the map.
			float ClosestToEdge = FMath::Max(FMath::Abs(XFalloff), FMath::Abs(YFalloff));

			// Set the Falloff map as a transition between the value closest to the edge and Transition.
			FalloffValues[Position] = Transition(ClosestToEdge, CurveStrength, FalloffBias);
		}
	}

	// The Falloff values, indexed by Y * XMap + X.
	return FalloffValues;
}

// (v ^ a) / (v ^ a + (b - bv) ^ a).
float UMapMaker::Transition(const float& V, const float& A, const float& B)
{
	// (v ^ a).
	float Numerator = FMath::Pow(V, A);

	// (b - bv) ^ a.
	float Denominator = FMath::Pow(B - B * V, A);

	// (v ^ a) / (v ^ a + (b - bv) ^ a).
	float Function = Numerator / (Numerator + Denominator);

	return Function;
}

// Generates Continents. (A large landmass, split by Water/Shallow).
TArray<float> UMapMaker::GenerateContinents()
{
	TArray<float> Continents;

	// Initialise to all zeroes.
	Continents.Init(0, XMap * YMap);

	float Offset = FMath::RandRange(-10000.f, 10000.f);
	UMW::Log("Continents Seed: " + FString::SanitizeFloat(Offset) + " at " + FString::SanitizeFloat(SplitRoughness) + " Roughness");

	for (int y = 0; y < YMap; ++y)
	{
		for (int x = 0; x < XMap; ++x)
		{
			int Position = y * XMap + x;

			// Get Perlin Noise value at these coordiantes.
			float Perlin = FMath::PerlinNoise2D(FVector2D(x + Offset, y + Offset) * SplitRoughness);

			// Clamp between 0 and 1.
			Perlin = (Perlin + 1) / 2;

			// If this Perlin Noise value qualifies as a split, assign it to Continents.
			if (Perlin < SplitLimit)
			{
				Continents[Position] = FMath::Clamp<float>(Perlin + SplitDistance, 0, 1);
			}
		}
	}

	// The Continent values, assigned as dips in the Map.
	return Continents;
}

// If X and Y are in the Map, given the relative Index.
bool UMapMaker::IsIndexInMapRange(const uint16& X, const uint16& Y, const uint16& Index) const
{
	int Position = Y * XMap + X;

	// If the Position is within Map.
	if (!((XMap * YMap) > Position))
	{
		return false;
	}

	// If you're on the X border.
	if (Index % XMap == XMap - 1)
	{
		// You can't go beyond the XMap or YMap.
		return !(X + 1 > XMap || Y + 1 > YMap);
	}

	if (Map.Num() != 0)
	{
		return (Map[Position]->Type != EType::MOUNTAIN && Map[Position]->Type != EType::WATER);
	}

	return true;
}

// Computes the Equatorial bounds of the map, according to Equator Influence.
// Do not use ABlock*'s to determine positions, this function is called before there are any blocks on the map.
// This method is purely numerical. Modify with confidence. Proceed with caution.
TArray<int> UMapMaker::ComputeEquator()
{
	TArray<int> Equator;

	// Initialise to all zeroes.
	Equator.Init(0, XMap * YMap);

	// NullIsland is the middle of Earth where the Prime Meridian intersects the Equator.
	int NullIsland = MapMidPoint();
	Equator.Add(NullIsland);

	// The Map's half X value.
	int MX = XMap / 2;
	// The Map's half Y value.
	int MY = YMap / 2;

	// Values at the vertical centre of the Equator.
	TArray<FVector2D> PrimeMeridian;
	PrimeMeridian.Add(FVector2D(MX, MY));

	float Offset = FMath::RandRange(-10000.f, 10000.f);
	UMW::Log("Equator Seed: " + FString::SanitizeFloat(Offset) + " at " + FString::SanitizeFloat(EquatorRoughness) + " Equator Roughness");
	
	// Compute the bounds North (and North West) of the Equator, limited by Equator Influence.
	for (int i = 0; i < EquatorInfluence; ++i)
	{
		// Offset the relative mid X and Y points by the Equator Influence for North West.
		int RelativeX = MX + i + 1;
		int RelativeY = MY + i + 1;
		int RelativeMid = RelativeY * XMap + RelativeX;

		// If these mid points are valid, add them to the Equator.
		if (IsIndexInMapRange(RelativeX, RelativeY, RelativeMid))
		{
			int NorthWest = RelativeY * XMap + RelativeX;
			PrimeMeridian.Add(FVector2D(RelativeX, RelativeY));
			Equator.Add(NorthWest);
		}

		// Offset the relative mid X and Y points by the Equator Influence for North.
		RelativeX = MX + i;
		RelativeY = MY + i + 1;
		RelativeMid = RelativeY * XMap + RelativeX;

		// If these mid points are valid, add them to the Equator.
		if (IsIndexInMapRange(RelativeX, RelativeY, RelativeMid))
		{
			int North = RelativeY * XMap + RelativeX;
			PrimeMeridian.Add(FVector2D(RelativeX, RelativeY));
			Equator.Add(North);
		}
	}

	// Compute the bounds South (and South East) of the Equator, limited by Equator Influence.
	for (int i = 0; i < EquatorInfluence; ++i)
	{
		// Offset the relative mid X and Y points by the Equator Influence for South East.
		int RelativeX = MX - i - 1;
		int RelativeY = MY - i - 1;
		int RelativeMid = RelativeY * XMap + RelativeX;

		// If these mid points are valid, add them to the Equator.
		if (IsIndexInMapRange(RelativeX, RelativeY, RelativeMid))
		{
			int SouthEast = RelativeY * XMap + RelativeX;
			PrimeMeridian.Add(FVector2D(RelativeX, RelativeY));
			Equator.Add(SouthEast);
		}

		// Offset the relative mid X and Y points by the Equator Influence for South.
		RelativeX = MX - i;
		RelativeY = MY - i - 1;
		RelativeMid = RelativeY * XMap + RelativeX;

		// If these mid points are valid, add them to the Equator.
		if (IsIndexInMapRange(RelativeX, RelativeY, RelativeMid))
		{
			int South = RelativeY * XMap + RelativeX;
			PrimeMeridian.Add(FVector2D(RelativeX, RelativeY));
			Equator.Add(South);
		}
	}

	// For every point in the Prime Meridian, go left and right and add them to the Equator.
	for (FVector2D NESW : PrimeMeridian)
	{
		// Right.
		// North East of the this point in the Prime Meridian.
		int X = NESW.X - 1;
		int Y = NESW.Y + 1;
		int R = Y * XMap + X;

		FVector2D NE = FVector2D(X, Y);

		// Keep going North East until the edge of the map.
		while (IsIndexInMapRange(NE.X, NE.Y, R))
		{
			int Position = NE.Y * XMap + NE.X;

			if (!Equator.Contains(Position))
			{
				float Perlin = FMath::PerlinNoise2D(FVector2D(NE.X + Offset, NE.Y + Offset) * EquatorRoughness);
				Perlin = (Perlin + 1) / 2;

				// If the Perlin Noise value qualifies as a point in the Equator, mark it as part of the Equator.
				if (Perlin - Transition(Perlin, EquatorStrength, EquatorBias) < EquatorSpread)
				{
					Equator.Add(Position);
				}
			}

			// Continue going North East.
			NE = FVector2D(NE.X - 1, NE.Y + 1);
			R = Position;
		}
		
		// Left.
		// South West of this point in the Prime Meridian.
		X = NESW.X + 1;
		Y = NESW.Y - 1;
		R = Y * XMap + X;

		FVector2D SW = FVector2D(X, Y);

		// Keep going South West until the edge of the map.
		while (IsIndexInMapRange(SW.X, SW.Y, R))
		{
			int Position = SW.Y * XMap + SW.X;

			if (!Equator.Contains(Position))
			{
				float Perlin = FMath::PerlinNoise2D(FVector2D(SW.X + Offset, SW.Y + Offset) * EquatorRoughness);
				Perlin = (Perlin + 1) / 2;

				// If the Perlin Noise value qualifies as a point in the Equator, mark it as part of the Equator.
				if (Perlin - Transition(Perlin, EquatorStrength, EquatorBias) < EquatorSpread)
				{
					Equator.Add(Position);
				}
			}

			// Continue going South West.
			SW = FVector2D(SW.X + 1, SW.Y - 1);
			R = Position;
		}
	}

	// The Index values of blocks that make up the Equator.
	return Equator;
}

