// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Block.h"
#include "MapMaker.generated.h"

class AWarrior;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class POLY_API UMapMaker : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMapMaker();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	static UMapMaker* Instance;

	ABlock* RandomBlock();
	TArray<ABlock*> Map;
	TArray<AWarrior*> AllWarriors;


	UPROPERTY(EditInstanceOnly)
		int XMap = 10;
	UPROPERTY(EditInstanceOnly)
		int YMap = 10;
	UPROPERTY(EditInstanceOnly)
		bool bUseFalloffMap;
	UPROPERTY(EditInstanceOnly)
		float FalloffBias;
	UPROPERTY(EditInstanceOnly)
		float CurveStrength;

	UPROPERTY(EditInstanceOnly, Category = Terrain)
		TSubclassOf<ABlock> Water;
	UPROPERTY(EditInstanceOnly, Category = Terrain)
		TSubclassOf<ABlock> Block;
	UPROPERTY(EditInstanceOnly, Category = Terrain)
		TSubclassOf<ABlock> Grass;
	UPROPERTY(EditInstanceOnly, Category = Terrain)
		TSubclassOf<ABlock> Stone;
	UPROPERTY(EditInstanceOnly, Category = Terrain)
		TSubclassOf<ABlock> Mountain;
	UPROPERTY(EditInstanceOnly, Category = Terrain)
		// Water | Grass | Stone | Mountain.
		FVector4 TypeLimits;
	UPROPERTY(EditInstanceOnly, Category = Terrain)
		float Roughness;

	UPROPERTY(EditInstanceOnly, Category = Players)
		TSubclassOf<AWarrior> Warrior;
	UPROPERTY(EditInstanceOnly, Category = Players)
		uint32 NumberOfWarriors;

private:

	void PlaceBlocks();
	ABlock* SpawnBlock(UClass*, const int&, const int&, EType);
	void ConnectBlocks();
	void SpawnWarriors();

	TArray<float> GenerateFalloffMap();

	//	If X and Y are in the ranges of Map.
	bool IsIndexInMapRange(const uint16& X, const uint16& Y, const uint16& Index) const;
};

UMapMaker* UMapMaker::Instance = nullptr;