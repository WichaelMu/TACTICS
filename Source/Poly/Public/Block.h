// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MW.h"
#include "Warrior.h"
#include "Containers/Queue.h"
#include "Block.generated.h"

class AMouseController;

UENUM()
enum class EType { WATER, GRASS, STONE, MOUNTAIN };

UCLASS()
class POLY_API ABlock : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABlock();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	UPROPERTY(VisibleAnywhere)
		int Index = -1;

	UPROPERTY(VisibleAnywhere)
		int AIAttacked;
	UPROPERTY(VisibleAnywhere)
		int HumanAttacked;

	// Neighbouring Blocks.
	UPROPERTY(VisibleAnywhere)
		ABlock* North = nullptr;
	UPROPERTY(VisibleAnywhere)
		ABlock* South = nullptr;
	UPROPERTY(VisibleAnywhere)
		ABlock* East = nullptr;
	UPROPERTY(VisibleAnywhere)
		ABlock* West = nullptr;

	UPROPERTY(VisibleAnywhere)
		ABlock* NorthEast = nullptr;
	UPROPERTY(VisibleAnywhere)
		ABlock* NorthWest = nullptr;
	UPROPERTY(VisibleAnywhere)
		ABlock* SouthEast = nullptr;
	UPROPERTY(VisibleAnywhere)
		ABlock* SouthWest = nullptr;

	UPROPERTY(VisibleAnywhere)
		// The Warrior that is currently occupying this Block.
		AWarrior* Occupant = nullptr;

	UFUNCTION(BlueprintCallable)
		void OnBlockClicked();

	// This block's world position.
	FVector GetWorldPosition() { return GetActorLocation(); }
	// This block's position for a warrior.
	FVector GetWarriorPosition() { return GetWorldPosition() + FVector(0, 0, 100); }
	// Mark this block as selected?
	void Selected(bool);

	//Pathfinding.
	// F score.
	float F;
	// G cost.
	float G;
	// H cost.
	float H;
	// Shortest path to origin via this block.
	ABlock* Parent;
	// Comparison of scores between other blocks.
	int CompareTo(ABlock*);

	// The type of terrain.
	EType Type;

	// Get this block's neighbour at orientation.
	ABlock* Get(uint8 Orientation) const;

	TArray<ABlock*> GetTraversableBlocks();
	TArray<ABlock*> SearchAtDepth(uint8 Range, const bool& bIgnoreOccupants = true);

	TArray<AWarrior*> SurroundingEnemiesInRange(EAffiliation);
	ABlock* GetClosestBlockToAHuman(TArray<ABlock*>);
	bool IsNextToAffiliation(const EAffiliation&);

	void DeductAttacks(EAffiliation);
	void AppendAttacks(EAffiliation);

private:

	void SearchDepthInitialise(TArray<ABlock*>&, uint8, const bool& bIgnoreOccupants = true);
	void SearchDepthLogic(TArray<ABlock*>& Blocks, uint8 Depth, TSet<ABlock*>& Visited, TQueue<ABlock*>& Breadth, const bool&);


};


