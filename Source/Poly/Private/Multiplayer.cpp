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

	AMapGenerator* MapGenerator = GetWorld()->SpawnActor<AMapGenerator>();

	if (MapGenerator)
	{
		MapGenerator->InitialiseGame();
	}
	else
	{
		UMW::LogError("AMultiplayer::No Map Generator!");
	}
}

void AMultiplayer::RegisterMovement(AWarrior& WarriorToMove, ABlock& BlockDestination)
{
	WarriorToMove.SetActorLocation(BlockDestination.GetWarriorPosition());
	UMW::Log("AMultiplayer::RegisterMovement Called");
}

void AMultiplayer::RegisterController(AController* Controller)
{
	APawn* Player = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, FVector::ZeroVector, FRotator(0, -65.f, 65.f));

	if (Controller && Player)
	{
		Controller->Possess(Player);
	}
}
