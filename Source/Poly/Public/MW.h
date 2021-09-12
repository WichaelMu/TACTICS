// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MW.generated.h"

class AWarrior;
class ABlock;
class UMoves;
class UWorld;

/**
 * Pathfinding.
 */
UCLASS()
class POLY_API UMW : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static bool Pathfind(ABlock*, ABlock*, TArray<ABlock*>&);
	static TArray<ABlock*> Pathfind(ABlock*, ABlock*);

	static void RunAI();
	static void Log(FString);

private:

	static void DetermineMoves();

};