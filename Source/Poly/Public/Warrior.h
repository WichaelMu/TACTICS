// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Warrior.generated.h"

class UStaticMeshComponent;
class ABlock;
class UWarriorHealthWidget;

UENUM(BlueprintType)
enum class EAffiliation : uint8 { HUMAN1, HUMAN2, AI };

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

	virtual bool ShouldTickIfViewportsOnly() const override;

	static int32 NumberOfAI;
	static int32 NumberOfHuman;
	static int32 EvaluateMap();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void OnSpawn(ABlock* SpawnedBlock, EAffiliation TeamAffiliation);
	UFUNCTION(Server, Reliable)
		void ServerOnSpawn(ABlock* SpawnedBlock, EAffiliation TeamAffiliation);


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USceneComponent* Root;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* RootMesh;

	void MoveTo(ABlock* TargetBlock);
	UFUNCTION(Server, Reliable, NetMulticast)
		void ServerMoveTo(ABlock* TargetBlock);
	UFUNCTION(BlueprintImplementableEvent)
		void RegisterMovement(ABlock* TargetBlock);
	UFUNCTION(BlueprintCallable)
	void UpdateBlock(ABlock* NewBlock);
	UFUNCTION(Server, Reliable, NetMulticast)
		void ServerUpdateBlock(ABlock* NewBlock);
	void UpdateBlockAttacks(ABlock* Departing, ABlock* Arriving);


	UPROPERTY(Replicated, VisibleAnywhere, Category = Health)
		int Health;
	void SetHealthText(const int& NewHealth);
	UPROPERTY(Replicated)
		UWarriorHealthWidget* HealthWidget;
	int Revive();
	// The damage this warrior will deal.
	const float Damage = 2;
	void DeductHealth();

	TArray<AWarrior*> GetAttackableWarriors();
	
	// The block this warrior is standing on.
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
		ABlock* CurrentBlock;
	UPROPERTY(Replicated)
		ABlock* PreviousBlock;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = Affiliation)
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


	// Retreat.
	ABlock* FindSafestBlock();

	// Attack.
	ABlock* FindKillableHuman();
	ABlock* Flank(ABlock*);

	// Search.
	ABlock* ConcentrationOfHumans();
	ABlock* ConcentrationOfAI();
	ABlock* FindNearestAffiliation(const EAffiliation& Nearest);
	TArray<ABlock*> CurrentPath;

	UFUNCTION(BlueprintCallable)
		void DealDamage();
	bool HealthIsFatal();
	void KillThisWarrior();

	ABlock* MoveTowardsBlock(ABlock* Relative);

};

int32 AWarrior::NumberOfAI = 0;
int32 AWarrior::NumberOfHuman = 0;

