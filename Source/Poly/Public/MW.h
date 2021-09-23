// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MW.generated.h"

class AWarrior;
class ABlock;
class UMoves;

/**
 * Pathfinding, AI and other helper functions for Poly.
 */
UCLASS()
class POLY_API UMW : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static bool Pathfind(ABlock* Origin, ABlock* Destination, TArray<ABlock*>& Path);
	static TArray<ABlock*> Pathfind(ABlock* Origin, ABlock* Destination);

	static void RunAI();
	static void Log(FString Message);

	static bool IsBlockTraversable(ABlock* Query);

private:

	static void DetermineMoves();

};