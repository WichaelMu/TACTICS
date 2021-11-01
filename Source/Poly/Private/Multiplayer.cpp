// Fill out your copyright notice in the Description page of Project Settings.


#include "Multiplayer.h"
#include "MapMaker.h"
#include "MW.h"
#include "MapGenerator.h"

void AMultiplayer::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// ...

	UMW::Log("Game Started...");
}