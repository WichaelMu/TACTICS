// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Multiplayer.generated.h"

class AWarrior;
class ABlock;

/**
 * 
 */
UCLASS()
class POLY_API AMultiplayer : public AGameMode
{
	GENERATED_BODY()
	
public:

	void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessages) override;

};
