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
		uint16 XMap;
	UPROPERTY(EditInstanceOnly)
		uint16 YMap;


	static void UpdatePosition(const FVector2D& Position);


protected:

	/*
	* Falloff settings.
	*/
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Falloff")
		bool bUseFalloffMap;
	/*Centre falloff modification. Higher = More land.*/
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Falloff")
		float FalloffBias;
	/*The smoothness of the transition. Higher = Sharper falloff edges.*/
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Falloff")
		float CurveStrength;

	/*
	* Continents settings.
	*/
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Continents")
		bool bGenerateContinents;
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Continents")
		float ContinentsSeed;
	/*The limit of Perlin Noise to begin splitting up the landmass. Higher = Large oceans.*/
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Continents", meta = (UIMin = "0", UIMax = "1", ClampMin = "0", ClampMax = "1"))
		float SplitLimit;
	/*How much should the splits take from the terrain. Higher = Larger splits.*/
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Continents", meta = (UIMin = "0", UIMax = "1", ClampMin = "0", ClampMax = "1"))
		float SplitDistance;
	/*How often should these splits occur. Higher = Spots of water.*/
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Continents", meta = (UIMin = "0", UIMax = "0.1", ClampMin = "0", ClampMax = "0.1"))
		float SplitScale;

	/*
	* Equator settings.
	*/
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Equator")
		bool bComputeEquatorialEnvironment;
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Equator")
		float EquatorSeed;
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Equator")
		int EquatorInfluence;
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Equator")
		float EquatorBias;
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Equator")
		float EquatorStrength;
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Equator")
		float EquatorSpread;
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Equator")
		float EquatorScale;
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Equator")
		TSubclassOf<ABlock> Desert;

	/*
	* Poisson Disc Sampling settings.
	*/
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Poisson Disc Sampling for Warrior Spawning")
		bool bGeneratePoissonSpawning;
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Poisson Disc Sampling for Warrior Spawning")
		uint16 IterationsBeforeRejection;
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Poisson Disc Sampling for Warrior Spawning")
		uint8 MinimumDistance;


	UPROPERTY(EditInstanceOnly, Category = Terrain)
		TSubclassOf<ABlock> Sand;
	UPROPERTY(EditInstanceOnly, Category = Terrain)
		TSubclassOf<ABlock> Shallow;
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
	
	// Water | Grass | Stone | Mountain, etc. limits.
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Limits")
		float WaterLimits;
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Limits")
		float ShallowLimits;
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Limits")
		float SandLimits;
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Limits")
		float GrassLimits;
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Limits")
		float StoneLimits;
	UPROPERTY(EditInstanceOnly, Category = "Terrain|Limits")
		float MountainLimits;

	UPROPERTY(EditInstanceOnly, Category = Terrain)
		float TerrainScale;
	UPROPERTY(EditInstanceOnly, Category = Terrain)
		float TerrainSeed;

	UPROPERTY(EditInstanceOnly, Category = Players)
		TSubclassOf<AWarrior> Warrior;
	UPROPERTY(EditInstanceOnly, Category = Players)
		int NumberOfWarriors;

private:

	static int MapMidPoint();

	void PlaceBlocks();
	void EvaluateBlockType(const float& Perlin, const int& X, const int& Y);
	ABlock* SpawnBlock(UClass* Class, const int& X, const int& Y, EType TerrainType);
	void ConnectBlocks();
	void SpawnWarriors();

	ABlock* GetPoissonOrRandomBlock();

	TArray<float> GenerateFalloffMap();
	TArray<float> GenerateContinents();
	static float Transition(const float& V, const float& A, const float& B);

	//	If X and Y are in the ranges of Map.
	bool IsIndexInMapRange(const uint16& X, const uint16& Y, const uint16& Index) const;

	TArray<int> ComputeEquator();


	float TerrainOffset;
	FVector2D CameraPosition;
	void UpdateChunks();
	uint16 XExtent;
	uint16 YExtent;
};

UMapMaker* UMapMaker::Instance = nullptr;
ABlock* UMapMaker::HumanConcentration = nullptr;
ABlock* UMapMaker::AIConcentration = nullptr;