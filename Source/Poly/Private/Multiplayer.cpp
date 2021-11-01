// Fill out your copyright notice in the Description page of Project Settings.


#include "Multiplayer.h"
#include "EngineUtils.h"
#include "MapMaker.h"
#include "MW.h"
#include "Warrior.h"
#include "Block.h"
#include "MapGenerator.h"

void AMultiplayer::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// ...

	UMW::Log("Game Started...");

	AMapGenerator* MapGenerator = nullptr;
	for (TActorIterator<AMapGenerator> it(GetWorld()); it; ++it)
	{
		MapGenerator = *it;
	}

	if (MapGenerator)
	{
		MapGenerator->InitialiseGame();
	}
	else
	{
		UMW::LogError("AMultiplayer::No Map Generator!");
	}

	UE_LOG(LogTemp, Error, TEXT("WHAT IS HAPPENING"));
}

void AMultiplayer::RegisterMovement(AWarrior& WarriorToMove, ABlock& BlockDestination)
{
	WarriorToMove.SetActorLocation(BlockDestination.GetWarriorPosition());
}
