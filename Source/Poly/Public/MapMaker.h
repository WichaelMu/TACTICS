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

	void GenerateBlocks();

	ABlock* RandomBlock();
	UPROPERTY(Replicated)
		TArray<ABlock*> Map;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
		TArray<AWarrior*> AllWarriors;


	UPROPERTY(Replicated, EditInstanceOnly)
		uint16 XMap;
	UPROPERTY(Replicated, EditInstanceOnly)
		uint16 YMap;


	static void UpdatePosition(const FVector2D& Position);

	void SpawnWarriors();
	UFUNCTION(Server, Reliable)
		void ServerSpawnWarriors();

	AWarrior* FindAuthorityWarrior(const AWarrior& InCompare);
	ABlock* FindAuthorityBlock(const ABlock&InCompare);
	UFUNCTION(Server, Reliable, NetMulticast)
		void UpdateAllBlocks();

	void RegisterWarrior(AWarrior* In, ABlock* Mark);

protected:

	/*
	* Falloff settings.
	*/
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Falloff")
		bool bUseFalloffMap;
	/*Centre falloff modification. Higher = More land.*/
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Falloff")
		float FalloffBias;
	/*The smoothness of the transition. Higher = Sharper falloff edges.*/
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Falloff")
		float CurveStrength;

	/*
	* Continents settings.
	*/
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Continents")
		bool bGenerateContinents;
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Continents")
		float ContinentsSeed;
	/*The limit of Perlin Noise to begin splitting up the landmass. Higher = Large oceans.*/
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Continents", meta = (UIMin = "0", UIMax = "1", ClampMin = "0", ClampMax = "1"))
		float SplitLimit;
	/*How much should the splits take from the terrain. Higher = Larger splits.*/
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Continents", meta = (UIMin = "0", UIMax = "1", ClampMin = "0", ClampMax = "1"))
		float SplitDistance;
	/*How often should these splits occur. Higher = Spots of water.*/
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Continents", meta = (UIMin = "0", UIMax = "0.1", ClampMin = "0", ClampMax = "0.1"))
		float SplitScale;

	/*
	* Equator settings.
	*/
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Equator")
		bool bComputeEquatorialEnvironment;
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Equator")
		float EquatorSeed;
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Equator")
		int EquatorInfluence;
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Equator")
		float EquatorBias;
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Equator")
		float EquatorStrength;
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Equator")
		float EquatorSpread;
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Equator")
		float EquatorScale;
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Equator")
		TSubclassOf<ABlock> Desert;

	/*
	* Poisson Disc Sampling settings.
	*/
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Poisson Disc Sampling for Warrior Spawning")
		bool bGeneratePoissonSpawning;
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Poisson Disc Sampling for Warrior Spawning")
		uint16 IterationsBeforeRejection;
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Poisson Disc Sampling for Warrior Spawning")
		uint8 MinimumDistance;


	UPROPERTY(Replicated, EditInstanceOnly, Category = Terrain)
		TSubclassOf<ABlock> Sand;
	UPROPERTY(Replicated, EditInstanceOnly, Category = Terrain)
		TSubclassOf<ABlock> Shallow;
	UPROPERTY(Replicated, EditInstanceOnly, Category = Terrain)
		TSubclassOf<ABlock> Water;
	UPROPERTY(Replicated, EditInstanceOnly, Category = Terrain)
		TSubclassOf<ABlock> Block;
	UPROPERTY(Replicated, EditInstanceOnly, Category = Terrain)
		TSubclassOf<ABlock> Grass;
	UPROPERTY(Replicated, EditInstanceOnly, Category = Terrain)
		TSubclassOf<ABlock> Stone;
	UPROPERTY(Replicated, EditInstanceOnly, Category = Terrain)
		TSubclassOf<ABlock> Mountain;
	
	// Water | Grass | Stone | Mountain, etc. limits.
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Limits")
		float WaterLimits;
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Limits")
		float ShallowLimits;
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Limits")
		float SandLimits;
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Limits")
		float GrassLimits;
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Limits")
		float StoneLimits;
	UPROPERTY(Replicated, EditInstanceOnly, Category = "Terrain|Limits")
		float MountainLimits;

	UPROPERTY(Replicated, EditInstanceOnly, Category = Terrain)
		float TerrainScale;
	UPROPERTY(Replicated, EditInstanceOnly, Category = Terrain)
		float TerrainSeed;

	UPROPERTY(Replicated, EditInstanceOnly, Category = Players)
		TSubclassOf<AWarrior> Warrior;
	UPROPERTY(Replicated, EditInstanceOnly, Category = Players)
		int NumberOfWarriors;


private:

	static int MapMidPoint();

	void PlaceBlocks();
	UFUNCTION(Server, Reliable)
		void ServerPlaceBlocks();
	void EvaluateBlockType(const float& Perlin, const int& X, const int& Y);
	void SpawnBlock(UClass* Class, const int& X, const int& Y, EType TerrainType);
	UFUNCTION(Server, Reliable)
		void ServerSpawnBlock(UClass* Class, const int& X, const int& Y, EType TerrainType);
	void ConnectBlocks();
	UFUNCTION(Server, Reliable)
		void ServerConnectBlocks();

	ABlock* GetPoissonOrRandomBlock();

	TArray<float> GenerateFalloffMap();
	TArray<float> GenerateContinents();
	static float Transition(const float& V, const float& A, const float& B);

	//	If X and Y are in the ranges of Map.
	bool IsIndexInMapRange(const uint16& X, const uint16& Y, const uint16& Index) const;

	TArray<int> ComputeEquator();


	UPROPERTY(Replicated)
		float TerrainOffset;
	UPROPERTY(Replicated)
		FVector2D CameraPosition;
	void UpdateChunks();
	UPROPERTY(Replicated)
		uint16 XExtent;
	UPROPERTY(Replicated)
		uint16 YExtent;

	void ClearBlocks();
};

UMapMaker* UMapMaker::Instance = nullptr;
ABlock* UMapMaker::HumanConcentration = nullptr;
ABlock* UMapMaker::AIConcentration = nullptr;
