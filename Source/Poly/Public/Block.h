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
class POLY_API ABlock : public AActor, public THeapItem<ABlock>, public TNode<ABlock>
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABlock();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:


	static TArray<ABlock*> ComputeTrajectory(ABlock* NearestHeuristic, uint8 Depth);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, VisibleAnywhere)
		int32 Index = -1;

	UPROPERTY(Replicated, VisibleAnywhere)
		int AIAttacked;
	UPROPERTY(Replicated, VisibleAnywhere)
		int HumanAttacked;

	// Neighbouring Blocks.
	UPROPERTY(Replicated, VisibleAnywhere)
		ABlock* North = nullptr;
	UPROPERTY(Replicated, VisibleAnywhere)
		ABlock* South = nullptr;
	UPROPERTY(Replicated, VisibleAnywhere)
		ABlock* East = nullptr;
	UPROPERTY(Replicated, VisibleAnywhere)
		ABlock* West = nullptr;

	UPROPERTY(Replicated, VisibleAnywhere)
		ABlock* NorthEast = nullptr;
	UPROPERTY(Replicated, VisibleAnywhere)
		ABlock* NorthWest = nullptr;
	UPROPERTY(Replicated, VisibleAnywhere)
		ABlock* SouthEast = nullptr;
	UPROPERTY(Replicated, VisibleAnywhere)
		ABlock* SouthWest = nullptr;

	UPROPERTY(Replicated, VisibleAnywhere)
		// The Warrior that is currently occupying this Block.
		AWarrior* Occupant = nullptr;

	UFUNCTION(BlueprintCallable)
		void OnBlockClicked();

	// This block's world position.
	FVector GetWorldPosition() override { return GetActorLocation(); }
	// This block's position for a warrior.
	UFUNCTION(BlueprintCallable)
		FVector GetWarriorPosition() { return GetWorldPosition() + FVector(0, 0, 100); }
	// Mark this block as selected?
	void Selected(bool);
	UFUNCTION(Server, Reliable, NetMulticast)
		void ServerSelected(bool bSelected);
	UFUNCTION(BlueprintImplementableEvent)
		void SetTraversableVisibility(bool bShow);


	// Comparison of scores between other blocks.
	int CompareTo(ABlock* Block) override;

	// The type of terrain.
	UPROPERTY(Replicated)
		EType Type;

	// Get this block's neighbour at orientation.
	ABlock* Get(const uint8& Orientation) const override;

	TArray<ABlock*> GetTraversableBlocks();
	TArray<ABlock*> SearchAtDepth(uint8 Range, const bool& bIgnoreOccupants = true);

	TArray<AWarrior*> SurroundingEnemiesInRange(EAffiliation RelativeTo);
	bool IsNextToAffiliation(const EAffiliation& RelativeTo);

	void DeductAttacks(EAffiliation DeductingAffiliation);
	void AppendAttacks(EAffiliation AppendingAffiliation);

private:

	void SearchDepthInitialise(TArray<ABlock*>& Blocks, uint8 Depth, const bool& bIgnoreOccupants = true);
	void SearchDepthLogic(TArray<ABlock*>& Blocks, uint8 Depth, TSet<ABlock*>& Visited, TQueue<ABlock*>& Breadth, const bool& bIgnoreOccupants);


	TArray<ABlock*> GetNeighbours() const;


};


