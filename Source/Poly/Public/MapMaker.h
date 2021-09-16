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
	static void GenerateLargestConcentrationOfHumans();
	static void GenerateLargestConcentrationOfAI();
	static ABlock* HumanConcentration;
	static ABlock* AIConcentration;

	ABlock* RandomBlock();
	TArray<ABlock*> Map;
	TArray<AWarrior*> AllWarriors;


	UPROPERTY(EditInstanceOnly)
		int XMap = 10;
	UPROPERTY(EditInstanceOnly)
		int YMap = 10;


protected:


	UPROPERTY(EditInstanceOnly, Category = "Terrain|Falloff")
		bool bUseFalloffMap;
	/*Centre falloff modification. Higher = More land.*/
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Falloff")
		float FalloffBias;
	/*The smoothness of the transition. Higher = Sharper falloff edges.*/
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Falloff")
		float CurveStrength;


	UPROPERTY(EditInstanceOnly, Category = "Terrain|Continents")
		bool bGenerateContinents;
	/*The limit of Perlin Noise to begin splitting up the landmass. Higher = Large oceans.*/
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Continents", meta = (UIMin = "0", UIMax = "1", ClampMin = "0", ClampMax = "1"))
		float SplitLimit;
	/*How much should the splits take from the terrain. Higher = Larger splits.*/
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Continents", meta = (UIMin = "0", UIMax = "1", ClampMin = "0", ClampMax = "1"))
		float SplitDistance;
	/*How often should these splits occur. Higher = Spots of water.*/
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Continents", meta = (UIMin = "0", UIMax = "0.1", ClampMin = "0", ClampMax = "0.1"))
		float SplitRoughness;


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
		int NumberOfWarriors;

private:

	static int MapMidPoint();

	void PlaceBlocks();
	ABlock* SpawnBlock(UClass*, const int&, const int&, EType);
	void ConnectBlocks();
	void SpawnWarriors();

	TArray<float> GenerateFalloffMap();
	TArray<float> GenerateContinents();
	static float Smooth(const float&, const float&, const float&);

	//	If X and Y are in the ranges of Map.
	bool IsIndexInMapRange(const uint16& X, const uint16& Y, const uint16& Index) const;
};

UMapMaker* UMapMaker::Instance = nullptr;
ABlock* UMapMaker::HumanConcentration = nullptr;
ABlock* UMapMaker::AIConcentration = nullptr;