// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PoissonDisc.generated.h"

class UMapMaker;
class ABlock;

/**
 * 
 */
UCLASS()
class POLY_API UPoissonDisc : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	

public:


	static void Flush();
	static ABlock* Sample(UMapMaker* MapMaker, const uint16& IterationsBeforeRejection, const uint8& MinimumDistance);
	static void Print();


private:


	static TSet<int32> Occupied;


};

