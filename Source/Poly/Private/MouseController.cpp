// Fill out your copyright notice in the Description page of Project Settings.


#include "MouseController.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "Warrior.h"
#include "Block.h"

// Sets default values
AMouseController::AMouseController()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Instance = this;
	
	CurrentTurn = EAffiliation::HUMAN;
}

// Called when the game starts or when spawned
void AMouseController::BeginPlay()
{
	Super::BeginPlay();

	MoveAmplifier = MinimumCameraMovementSpeed * 4;
}

// Called to bind functionality to input
void AMouseController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Forward", this, &AMouseController::Forward);
	PlayerInputComponent->BindAxis("Right", this, &AMouseController::Right);

	PlayerInputComponent->BindAxis("Scroll", this, &AMouseController::Throttle);
}

// Called un ABlock::OnBlockClicked.
/// <summary>What is done when an ABlock is clicked.</summary>
/// <param name="ClickedBlock">The ABlock that was clicked.</param>
void AMouseController::BlockClicked(ABlock* ClickedBlock)
{
	// Called from ABlock::OnBlockClicked().
	// Also called from ABlock::Blueprint::OnMouseClicked.
	if (Instance)
	{
		Instance->CallByBlock(ClickedBlock);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("THERE IS NO AMOUSECONTROLLER::INSTANCE!"))
	}
}

void AMouseController::EndTurn()
{
	if (CurrentTurn == EAffiliation::HUMAN)
	{
		CurrentTurn = EAffiliation::AI;

		// After making moves, clear traversable[?] and end turn.
		UMW::RunAI();
		EndTurn();
	}
	else
	{
		CurrentTurn = EAffiliation::HUMAN;
	}

	AlreadyMovedWarriors.Empty();
}

// Called by Forward Axis.
/// <summary>Controls the forward/backward movement of the player.</summary>
/// <param name="Throw"></param>
void AMouseController::Forward(float Throw)
{
	FRotator ActorForwardYaw = GetControlRotation();
	ActorForwardYaw.Pitch = 0;
	ActorForwardYaw.Roll = 0;

	SetActorLocation(GetActorLocation() + ActorForwardYaw.Vector() * Throw * MoveAmplifier * GetWorld()->GetDeltaSeconds());
}

// Called by Right Axis.
/// <summary>Controls the left/right movement of the player.</summary>
/// <param name="Throw"></param>
void AMouseController::Right(float Throw)
{
	SetActorLocation(GetActorLocation() + GetActorRightVector() * Throw * MoveAmplifier * GetWorld()->GetDeltaSeconds());
}

// Called by Scroll Axis.
/// <summary>Controls the movement speed of the player.</summary>
/// <param name="Throw"></param>
void AMouseController::Throttle(float Throw)
{
	MoveAmplifier += Throw * 100;
	// Clamp the movement speed to a minimum of 1.
	if (MoveAmplifier < MinimumCameraMovementSpeed)
	{
		MoveAmplifier = MinimumCameraMovementSpeed;
	}
}


// What happens when a block is clicked?
void AMouseController::CallByBlock(ABlock* ClickedBlock)
{
	// If the most recently clicked block is not the previously clicked block.
	if (CurrentlySelectedBlock != ClickedBlock)
	{
		// If this block is entered, it means that this is the second click.
		// (That is, a click while something was clicked on before.)

		// Do what needs to be done on the second press.
		LMBPressed(ClickedBlock);
		
		if (CurrentlySelectedBlock)
		{
			CurrentlySelectedBlock = ClickedBlock;
		}
	}
	else // If the clicked block is the same as the already clicked block.
	{
		// Mark it for deselection.
		CurrentlySelectedBlock->Selected(false);
		// Reset the currently selected block.
		CurrentlySelectedBlock = nullptr;

		ClearTraversable();
	}
}

/// <summary>What happens if an ABlock is clicked?</summary>
/// <param name="ClickedBlock">The ABlock that was clicked.</param>
void AMouseController::LMBPressed(ABlock* ClickedBlock)
{
	// If a block is already selected.
	if (CurrentlySelectedBlock)
	{
		// Mark the block for deselection.
		CurrentlySelectedBlock->Selected(false);
		
		if (CurrentlySelectedWarrior)
		{
			if (!ClickedBlock->Occupant)
			{
				if (Traversable.Contains(ClickedBlock) && !AlreadyMovedWarriors.Contains(CurrentlySelectedWarrior))
				{
					CurrentlySelectedWarrior->MoveTo(ClickedBlock);
					AlreadyMovedWarriors.Add(CurrentlySelectedWarrior);
				}
			}
		}

		ClearTraversable();

		// Reset the currently selected block.
		CurrentlySelectedBlock = nullptr;
	}
	else // If there is no selected block.
	{
		// Don't do anything if the clicked block is NOT occupied.
		if (ClickedBlock->Occupant)
		{
			// If the clicked block's occupant is the on the current turn's affiliation.
			if (ClickedBlock->Occupant->Affiliation == CurrentTurn)
			{
				if (!AlreadyMovedWarriors.Contains(ClickedBlock->Occupant))
				{
					// Set the selected block to the clicked block.
					CurrentlySelectedBlock = ClickedBlock;
					// Mark it for selection.
					CurrentlySelectedBlock->Selected(true);

					// Select this block's warrior.
					CurrentlySelectedWarrior = ClickedBlock->Occupant;
				}
			}
		}
	}
}

void AMouseController::ClearTraversable()
{
	// Deselect all traversable blocks. Maximum is 25.
	for (ABlock* Allowed : Traversable)
	{
		Allowed->Selected(false);
	}

	Traversable.Empty();
}

