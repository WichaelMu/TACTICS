// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGenerator.h"
#include "Components/SceneComponent.h"
#include "MapMaker.h"
#include "MW.h"
#include "WarriorSpawner.h"

// Sets default values
AMapGenerator::AMapGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// ...
	
	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Map Generator"));
	RootComponent = SceneComponent;

	MapMaker = CreateDefaultSubobject<UMapMaker>(TEXT("Map Maker Logic"));

	if (MapMaker)
	{
		AddOwnedComponent(MapMaker);
		MapMaker->SetIsReplicated(true);
	}
}


void AMapGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ...

	// This is obsolete. Map generation is now done on entry.
	/*if (bGenerateMap)
	{
		if (MapMaker)
		{
			MapMaker->GenerateBlocks();
		}
		else
		{
			UMW::LogError("AMapGenerator::Tick No Map Maker!");
		}

		bGenerateMap = !bGenerateMap;
	}*/
}


bool AMapGenerator::ShouldTickIfViewportsOnly() const
{
	return false;
}

void AMapGenerator::InitialiseGame()
{
	ServerInitialiseGame();

	Instance = this;
}

// This is where the game begins to take place.
void AMapGenerator::ServerInitialiseGame_Implementation()
{
	if (MapMaker && GetLocalRole() == ROLE_Authority)
	{
		const float TerrainSeed = FMath::RandRange(-10000.f, 10000.f);
		const float ContinentsSeed = FMath::RandRange(-10000.f, 10000.f);
		const float EquatorSeed = FMath::RandRange(-10000.f, 10000.f);

		MapMaker->GenerateBlocks(TerrainSeed, ContinentsSeed, EquatorSeed);
	}
}

