// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Warrior.generated.h"

class UStaticMeshComponent;
class ABlock;

UENUM()
enum class EAffiliation : uint8 { HUMAN, AI };

UCLASS()
class POLY_API AWarrior : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWarrior();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	static int32 NumberOfAI;
	static int32 NumberOfHuman;
	static int32 EvaluateMap();


	void OnSpawn(ABlock*, EAffiliation);


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USceneComponent* Root;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* RootMesh;

	void MoveTo(ABlock*);
	void UpdateBlock(ABlock*);
	void UpdateBlockAttacks(ABlock*, ABlock*);


	UPROPERTY(VisibleAnywhere, Category = Health)
		int Health;
	int Revive();
	// The damage this warrior will deal.
	const float Damage = 2;
	void DeductHealth();

	TArray<AWarrior*> GetAttackableWarriors();
	
	// The block this warrior is standing on.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		ABlock* CurrentBlock;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Affiliation)
		EAffiliation Affiliation;

	// Defined in blueprint.
	UFUNCTION(BlueprintImplementableEvent)
		void AssignAffiliationColours();


	void Retreat();
	void Attack();
	void Search();


protected:


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Affiliation)
		FColor AffiliateColour1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Affiliation)
		FColor AffiliateColour2;


private:
	const FVector WarriorScale = FVector(.5f, .5f, 1.5f);


	FVector DirectionToPath;
	void MoveTowards();

	// Retreat.
	ABlock* FindSafestBlock();

	// Attack.
	ABlock* FindKillableHuman();

	// Search.
	ABlock* MoveTowardsConcentrationOfHumans();
	ABlock* MoveTowardsConcentrationOfAI();
	ABlock* FindNearestAffiliation(const EAffiliation&);
	TArray<ABlock*> CurrentPath;

	void DealDamage();
	bool HealthIsFatal();
	void KillThisWarrior();

	ABlock* MoveTowardsBlock(ABlock*);

};

int32 AWarrior::NumberOfAI = 0;
int32 AWarrior::NumberOfHuman = 0;

