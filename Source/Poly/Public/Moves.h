// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Moves.generated.h"

class AWarrior;
class ABlock;

/**
 * 
 */
UCLASS()
class POLY_API UMoves : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	AWarrior* Warrior;
	ABlock* Block;

	static UMoves* MakeMove(AWarrior*, ABlock*);

};
