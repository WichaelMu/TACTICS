// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGenerator.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "MapMaker.h"
#include "MW.h"

// Sets default values
AMapGenerator::AMapGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// ...
	
	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Map Generator"));
	RootComponent = SceneComponent;

	MapMaker = CreateDefaultSubobject<UMapMaker>(TEXT("Map Maker Logic"));

	if (MapMaker)
	{
		AddOwnedComponent(MapMaker);
	}
	else
	{
		UMW::LogError("AMapGenerator::Cannot add Map Maker!");
	}
}


void AMapGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ...

	if (bGenerateMap)
	{
		if (MapMaker)
		{
			MapMaker->GenerateBlocks();
		}
		else
		{
			UMW::LogError("AMapGenerator::No Map Maker!");
		}

		bGenerateMap = !bGenerateMap;
	}
}


bool AMapGenerator::ShouldTickIfViewportsOnly() const
{
	return true;
}

