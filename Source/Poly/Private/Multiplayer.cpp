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

	// Create a MapGenerator.
	AMapGenerator* MapGenerator = GetWorld()->SpawnActor<AMapGenerator>();

	// Make the call to begin the game.
	if (MapGenerator)
	{
		MapGenerator->InitialiseGame();
	}
	else
	{
		UMW::LogError("AMultiplayer::No Map Generator!");
	}

	// Spawn the players. Otherwise, they won't spawn and might crash, for some reason.
	APawn* P1 = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, FVector::ZeroVector, FRotator(0, -65.f, 65.f));
	APawn* P2 = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, FVector::ZeroVector, FRotator(0, -65.f, 65.f));
}

// Register a Warrior's movement.
void AMultiplayer::RegisterMovement(AWarrior* WarriorToMove, ABlock* BlockDestination)
{
	WarriorToMove->SetActorLocation(BlockDestination->GetWarriorPosition());

	WarriorToMove->PreviousBlock = WarriorToMove->CurrentBlock;

	WarriorToMove->CurrentBlock->Occupant = nullptr;

	UMW::Log("AMultplayer::RegisterMovement Passed UpdateBlockAttacks()");
	//UpdateBlockAttacks(CurrentBlock, NewBlock);

	if (BlockDestination)
	{
		BlockDestination->Occupant = WarriorToMove;
	}
	else
	{
		UMW::LogError("AMultiplayer::RegisterMovement No BlockDestination");
	}

	WarriorToMove->CurrentBlock = BlockDestination;
	
	UMW::Log("AMultiplayer::RegisterMovement Called");
}

// Ensure the player correctly possesses the Characters.
void AMultiplayer::RegisterController(AController* Controller)
{
	APawn* Player = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, FVector::ZeroVector, FRotator(0, -65.f, 65.f));

	if (Controller && Player)
	{
		Controller->Possess(Player);
	}
}
