// Fill out your copyright notice in the Description page of Project Settings.


#include "MouseController.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "Warrior.h"
#include "Block.h"
#include "MapMaker.h"
#include "Net/UnrealNetwork.h"
#include "Multiplayer.h"


// Sets default values
AMouseController::AMouseController()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Instance = this;
	
	CurrentTurn = EAffiliation::HUMAN1;
}


// Called when the game starts or when spawned
void AMouseController::BeginPlay()
{
	Super::BeginPlay();

	MoveAmplifier = MinimumCameraMovementSpeed * 4;

	SetActorRotation(FRotator(-65.f, 65.f, 0.f));

	if (GetLocalRole() == ROLE_Authority)
	{
		auto GameMode = GetWorld()->GetAuthGameMode();
		if (GameMode)
		{
			AMultiplayer* MultiplayerGameMode = Cast<AMultiplayer>(GameMode);

			if (MultiplayerGameMode)
			{
				Multiplayer = MultiplayerGameMode;
				UMW::Log("Found MP GM");
			}
			else
			{
				UMW::LogError("AMouseController::BeginPlay No MultiplayerGameMode");
			}
		}
		else
		{
			UMW::LogError("AMouseController::BeginPlay No GameMode");
		}
	}
	else
	{
		UMW::LogError("AMouseController::BeginPlay Not Authority");
	}
}


// Called to bind functionality to input
void AMouseController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	
	// WASD.
	PlayerInputComponent->BindAxis("Forward", this, &AMouseController::Forward);
	PlayerInputComponent->BindAxis("Right", this, &AMouseController::Right);

	// Scroll for camera movement speed.
	PlayerInputComponent->BindAxis("Scroll", this, &AMouseController::Throttle);

	// Spacebar / LShift for altitude.
	PlayerInputComponent->BindAxis("Rise", this, &AMouseController::Rise);
}

void AMouseController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// ...

	DOREPLIFETIME(AMouseController, Traversable);
	DOREPLIFETIME(AMouseController, MoveAmplifier);
	DOREPLIFETIME(AMouseController, Multiplayer);
	DOREPLIFETIME(AMouseController, CurrentlySelectedBlock);
	DOREPLIFETIME(AMouseController, CurrentlySelectedWarrior);
	DOREPLIFETIME(AMouseController, AlreadyMovedWarriors);
}


// Called by Forward Axis.
/// <summary>Controls the forward/backward movement of the player.</summary>
/// <param name="Throw"></param>
void AMouseController::Forward(float Throw)
{
	ServerForward(Throw);
}


// Called by Right Axis.
/// <summary>Controls the left/right movement of the player.</summary>
/// <param name="Throw"></param>
void AMouseController::Right(float Throw)
{
	ServerRight(Throw);
}


// Called by Scroll Axis.
/// <summary>Controls the movement speed of the player.</summary>
/// <param name="Throw"></param>
void AMouseController::Throttle(float Throw)
{
	ServerThrottle(Throw);
}


void AMouseController::Rise(float Throw)
{
	ServerRise(Throw);
}

void AMouseController::ServerForward_Implementation(const float& Throw)
{
	FRotator ActorForwardYaw = GetControlRotation();
	ActorForwardYaw.Pitch = 0;
	ActorForwardYaw.Roll = 0;

	FVector Position = GetActorLocation();
	SetActorLocation(Position + ActorForwardYaw.Vector() * Throw * MoveAmplifier * GetWorld()->GetDeltaSeconds());

	UMapMaker::UpdatePosition(FVector2D(Position.X, Position.Y));
}

void AMouseController::ServerRight_Implementation(const float& Throw)
{
	FVector Position = GetActorLocation();
	SetActorLocation(Position + GetActorRightVector() * Throw * MoveAmplifier * GetWorld()->GetDeltaSeconds());

	UMapMaker::UpdatePosition(FVector2D(Position.X, Position.Y));
}

void AMouseController::ServerThrottle_Implementation(const float& Throw)
{
	MoveAmplifier += Throw * 100;
	// Clamp the movement speed to a minimum of 1.
	if (MoveAmplifier < MinimumCameraMovementSpeed)
	{
		MoveAmplifier = MinimumCameraMovementSpeed;
	}
}

void AMouseController::ServerRise_Implementation(const float& Throw)
{
	SetActorLocation(GetActorLocation() + FVector::UpVector * Throw * MoveAmplifier * GetWorld()->GetDeltaSeconds());
}


void AMouseController::EndTurn()
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		// Ending Human turn.
		if (CurrentTurn == EAffiliation::HUMAN1) // Ending P1 turn.
		{
			UMW::Log("Ending P1");

			// What happens when the Human ends their turn.

			// Move on to P2.
			CurrentTurn = EAffiliation::HUMAN2;
		}
		else if (CurrentTurn == EAffiliation::HUMAN2) // Ending P2 turn.
		{
			UMW::Log("Ending P2");

			// Move on to AI.
			CurrentTurn = EAffiliation::AI;

			UMapMaker::GenerateLargestConcentrationOfHumans();
			UMapMaker::GenerateLargestConcentrationOfAI();

			// After making moves, clear traversable[?] and end turn.
			UMW::RunAI();

			// After the AI has made its moves, end the turn.
			EndTurn();
		}
		else // Ending AI turn.
		{
			UMW::Log("Ending AI");

			// What happens when the AI ends their turn.

			CurrentTurn = EAffiliation::HUMAN1;

		}
	}

	AlreadyMovedWarriors.Empty();
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

void AMouseController::OnBlockClicked(ABlock* ClickedBlock)
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		CallByBlock(ClickedBlock);
	}
}


// What happens when a block is clicked?
void AMouseController::CallByBlock(ABlock* ClickedBlock)
{
	ServerCallByBlock(ClickedBlock);
}


void AMouseController::ServerCallByBlock_Implementation(ABlock* ClickedBlock)
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
	ServerLMBPressed(ClickedBlock);
}


void AMouseController::ServerLMBPressed_Implementation(ABlock* ClickedBlock)
{
	UMW::Log("1: Execution begins.");
	// If a block is already selected.
	if (CurrentlySelectedBlock)
	{
		UMW::Log("2: A Block is already selected. Marked for deselection.");
		// Mark the block for deselection.
		CurrentlySelectedBlock->Selected(false);

		if (CurrentlySelectedWarrior)
		{
			UMW::Log("3: A Warrior is already selected.");
			if (!ClickedBlock->Occupant)
			{
				UMW::Log("4: This Block does NOT have an occupant.");
				// If the clicked block is traversable and the selected warrior has not already moved.
				if (Traversable.Contains(ClickedBlock) && !AlreadyMovedWarriors.Contains(CurrentlySelectedWarrior))
				{
					UMW::Log("5: This block is in Traversable and this Warrior has not moved. Moving.");
					
					// Allow movement.
					CurrentlySelectedWarrior->MoveTo(ClickedBlock);

					// Mark the selected warrior as moved.
					AlreadyMovedWarriors.Add(CurrentlySelectedWarrior);
				}
			}
		}
		UMW::Log("6: Clearing Traversable and nullifying selection.");
		// Disallow any other warriors from moving to a previously traversable block.
		ClearTraversable();

		// Reset the currently selected block.
		CurrentlySelectedBlock = nullptr;
	}
	else // If there is no selected block.
	{
		UMW::Log("7: No Block selected.");
		// Don't do anything if the clicked block is NOT occupied.
		if (ClickedBlock->Occupant)
		{
			UMW::Log("8: This Block IS occupied.");
			// If the clicked block's occupant is the on the current turn's affiliation.
			if (ClickedBlock->Occupant->Affiliation == CurrentTurn)
			{
				UMW::Log("9: The occupied Warrior is the Current Turn.");
				if (!AlreadyMovedWarriors.Contains(ClickedBlock->Occupant))
				{
					UMW::Log("10: This Warrior has NOT moved. Marking for selection.");
					// Set the selected block to the clicked block.
					CurrentlySelectedBlock = ClickedBlock;
					// Mark it for selection.
					CurrentlySelectedBlock->Selected(true);
					Traversable = CurrentlySelectedBlock->GetTraversableBlocks();

					// Select this block's warrior.
					CurrentlySelectedWarrior = ClickedBlock->Occupant;
				}
			}
		}
	}

	UMW::Log("OUT");
}

void AMouseController::ClearTraversable()
{
	ServerClearTraversable();
}

void AMouseController::ServerClearTraversable_Implementation()
{
	// Deselect all traversable blocks. Maximum is 25.
	for (ABlock* Allowed : Traversable)
	{
		Allowed->Selected(false);
	}

	Traversable.Empty();
}

