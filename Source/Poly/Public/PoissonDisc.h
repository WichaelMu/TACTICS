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
	static ABlock* Sample(UMapMaker*, const uint16&, const uint8&);

private:


	static TSet<int32> Occupied;


};

