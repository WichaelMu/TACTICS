// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Warrior.h"
#include "Net/UnrealNetwork.h"
#include "MouseController.generated.h"

class ABlock;

/**
* Handles the clicking of the mouse, selection of blocks and *not yet* controls the movement.
*/
UCLASS()
class POLY_API AMouseController : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMouseController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	static void BlockClicked(ABlock*);
	static AMouseController* Instance;
	static EAffiliation CurrentTurn;

	UPROPERTY(Replicated)
		TArray<ABlock*> Traversable;


	UFUNCTION(BlueprintCallable)
		void EndTurn();


private:

	void Forward(float Throw);
	void Right(float Throw);

	float MoveAmplifier;
	void Throttle(float Throw);

	void Rise(float Throw);

	UFUNCTION(Server, Reliable)
		void ServerForward(const float& Throw);
	UFUNCTION(Server, Reliable)
		void ServerRight(const float& Throw);
	UFUNCTION(Server, Reliable)
		void ServerThrottle(const float& Throw);
	UFUNCTION(Server, Reliable)
		void ServerRise(const float& Throw);

	// Clamp the movement speed of the camera to this speed.
	UPROPERTY(EditAnywhere)
		float MinimumCameraMovementSpeed;

	void CallByBlock(ABlock* ClickedBlock);
	void LMBPressed(ABlock* ClickedBlock);
	void ClearTraversable();

	ABlock* CurrentlySelectedBlock = nullptr;
	AWarrior* CurrentlySelectedWarrior = nullptr;
	
	TSet<AWarrior*> AlreadyMovedWarriors;
};


AMouseController* AMouseController::Instance = nullptr;
EAffiliation AMouseController::CurrentTurn = EAffiliation::HUMAN1;