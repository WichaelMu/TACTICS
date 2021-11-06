// Fill out your copyright notice in the Description page of Project Settings.


#include "Multiplayer.h"
#include "MW.h"
#include "MapGenerator.h"

void AMultiplayer::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// ...

	UMW::Log("Game Started...");

	// Create a MapGenerator.
	AMapGenerator* MapGenerator = GetWorld()->SpawnActor<AMapGenerator>();

	// Make the call to begin the game.
	if (MapGenerator)
	{
		MapGenerator->InitialiseGame();
	}

	// Spawn the players. Otherwise, they won't spawn and might crash, for some reason.
	APawn* P1 = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, FVector::ZeroVector, FRotator(0, -45.f, 45.f));
	APawn* P2 = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, FVector::ZeroVector, FRotator(0, -45.f, 45.f));
}
