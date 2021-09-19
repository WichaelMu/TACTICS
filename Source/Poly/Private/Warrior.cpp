// Fill out your copyright notice in the Description page of Project Settings.


#include "Warrior.h"
#include "Block.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "MapMaker.h"
#include "MW.h"
#include "TimerManager.h"

// Sets default values
AWarrior::AWarrior()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;


	// Make a new cube root component.
	UBoxComponent* Cube = CreateDefaultSubobject<UBoxComponent>(TEXT("Root Component"));
	RootComponent = Cube;
	Cube->InitBoxExtent(WarriorScale);
	Root = Cube;

	// Assign a new mesh to the root so that we can see it.
	UStaticMeshComponent* CubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube"));
	CubeMesh->AttachTo(Cube);
	RootMesh = CubeMesh;
	
	// Set the static mesh for the root.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BoxMesh(TEXT("/Game/StarterContent/Shapes/Shape_Cube.Shape_Cube"));
	if (BoxMesh.Succeeded())
	{
		CubeMesh->SetStaticMesh(BoxMesh.Object);
		CubeMesh->SetRelativeLocation(FVector::ZeroVector);
		CubeMesh->SetWorldScale3D(WarriorScale);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("COULD NOT FIND WARRIOR CUBE MESH!"));
	}
	
}

// Called when the game starts or when spawned
void AWarrior::BeginPlay()
{
	Super::BeginPlay();
	

	Health = 20;
}

// Called every frame
void AWarrior::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Positive = AI advantage. Negative = Human advantage. 0 = Equal material.
int32 AWarrior::EvaluateMap()
{
	return NumberOfAI - NumberOfHuman;
}

// When this warrior is spawned into the world.
void AWarrior::OnSpawn(ABlock* SpawnedBlock, EAffiliation TeamAffiliation)
{
	this->Affiliation = TeamAffiliation;
	if (SpawnedBlock)
	{
		CurrentBlock = SpawnedBlock;

		// Change the location of this warrior.
		SetActorLocation(SpawnedBlock->GetWarriorPosition());
		UpdateBlock(SpawnedBlock);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("NO SPAWNED BLOCK."));
	}
}

// Move this warrior to TargetBlock.
void AWarrior::MoveTo(ABlock* TargetBlock)
{
	// TODO: Change the location of the warrior with interpolation and pathfinding.
	if (CurrentBlock)
	{
		if (CurrentBlock != TargetBlock)
		{
			//FTimerManager TimerManager;
			//FTimerHandle TimerHandle;

			//TArray<ABlock*> Path = UMW::Pathfind(CurrentBlock, TargetBlock);
			
			//for (int i = Path.Num() - 1; i >= 0; --i)
			//{
			//	/*FVector WarriorLocation = GetActorLocation();
			//	FVector PathLocation = Path[i]->GetWarriorPosition();
			//	DirectionToPath = (PathLocation - WarriorLocation).GetSafeNormal();*/
			//	
			//	if (i > 0)
			//	{
			//		//DrawDebugLine(GetWorld(), Path[i]->GetWarriorPosition(), Path[i - 1]->GetWarriorPosition(), FColor::White, true, 10, 0U, 10);
			//	}
			//}

			//SetActorLocation(Path[0]->GetWarriorPosition());

			//Path.Empty();
			
			SetActorLocation(TargetBlock->GetWarriorPosition());
		}
	}

	UpdateBlock(TargetBlock);
	DealDamage();
}

// Update block information when moving between blocks.
void AWarrior::UpdateBlock(ABlock* NewBlock)
{
	CurrentBlock->Occupant = nullptr;

	UpdateBlockAttacks(CurrentBlock, NewBlock);

	NewBlock->Occupant = this;
	CurrentBlock = NewBlock;
}

// Update the attack references on the block.
void AWarrior::UpdateBlockAttacks(ABlock* From, ABlock* To)
{
	if (From != To)
	{
		From->DeductAttacks(Affiliation);
	}

	To->AppendAttacks(Affiliation);
}

// Health some health.
int AWarrior::Revive()
{
	int NewHealth = FMath::Max<int>(Health + 3, 20);
	Health = NewHealth;
	return NewHealth;
}

// Has the health fallen below zero?
bool AWarrior::HealthBelowZero()
{
	return Health <= 0;
}

// Disassociate this warrior as part of Poly.
void AWarrior::KillThisWarrior()
{
	// Deregister this Warrior as part of all warriors.
	UMapMaker::Instance->AllWarriors.Remove(this);
	
	// Move this warrior somewhere out of the screen.
	SetActorLocation(FVector(40000.f));

	CurrentBlock->DeductAttacks(Affiliation);
	CurrentBlock->Occupant = nullptr;
	CurrentBlock = nullptr;

	if (Affiliation == EAffiliation::HUMAN)
	{
		NumberOfHuman--;
	}
	else
	{
		NumberOfAI--;
	}
}

ABlock* AWarrior::MoveTowardsBlock(ABlock* Relative)
{
	CurrentPath = UMW::Pathfind(CurrentBlock, Relative);
	if (CurrentPath.Num() > 1)
	{
		return CurrentPath.Last(1);
	}

	ABlock* Towards = CurrentBlock;

	float Closest = INT_MAX;
	FVector RelativePosition = Relative->GetWarriorPosition();
	TArray<ABlock*> Traversable = CurrentBlock->GetTraversableBlocks();

	for (int i = 0; i < Traversable.Num(); ++i)
	{
		ABlock* Block = Traversable[i];
		float ClosingDistance = FVector::DistSquared(Block->GetWarriorPosition(), RelativePosition);

		if (ClosingDistance < Closest)
		{
			Closest = ClosingDistance;
			Towards = Block;
		}
	}

	CurrentPath.Empty();
	return Towards;
}

// Take damage.
void AWarrior::DeductHealth()
{
	Health -= Damage;

	if (HealthBelowZero())
	{
		KillThisWarrior();
	}
}

// Gets every ABlock around CurrentBlock if it is occupied.
TArray<AWarrior*> AWarrior::GetAttackableWarriors()
{
	TArray<AWarrior*> Attackable;

	if (CurrentBlock)
	{
		for (int32 i = 0; i < 8; ++i)
		{
			AWarrior* SurroundingOccupant = CurrentBlock->Get(i)->Occupant;
			if (SurroundingOccupant)
			{
				if (SurroundingOccupant->Affiliation != Affiliation)
				{
					Attackable.Add(SurroundingOccupant);
				}
			}
		}
	}

	return Attackable;
}

void AWarrior::Retreat()
{
	MoveTo(FindSafestBlock());
}

void AWarrior::Attack()
{
	MoveTo(FindKillableHuman());
}

void AWarrior::Search()
{
	MoveTo(MoveTowardsConcentrationOfHumans());
}

ABlock* AWarrior::FindSafestBlock()
{
	TArray<ABlock*> Traversable = CurrentBlock->GetTraversableBlocks();

	ABlock* MaximumAI = CurrentBlock;
	ABlock* MinimalHumans = CurrentBlock;

	int MaxAI = INT_MIN;
	int MinHuman = INT_MAX;

	for (ABlock* Block : Traversable)
	{
		if (Block->AIAttacked > MaxAI)
		{
			MaxAI = Block->AIAttacked;
			MaximumAI = Block;
		}

		if (Block->HumanAttacked < MinHuman)
		{
			MinHuman = Block->HumanAttacked;
			MinimalHumans = Block;
		}
	}
	
	return MaxAI > MinHuman ? MaximumAI : MinimalHumans;
}

ABlock* AWarrior::FindKillableHuman()
{
	// AI has already won.
	if (NumberOfHuman == 0)
	{
		UMW::Log("AI HAS WON.");
		return MoveTowardsConcentrationOfHumans(); // Will default to centre.
	}

	// If this block is overwhelmed by humans, run away || stay out of range.
	if (CurrentBlock->AIAttacked < CurrentBlock->HumanAttacked)
	{
		return FindSafestBlock();
	}

	TArray<ABlock*> Traversable = CurrentBlock->GetTraversableBlocks();

	TArray<ABlock*> DesirableBlocks;

	for (ABlock* Block : Traversable)
	{
		// Find a block where HumanAttacked > 0:
		if (Block->HumanAttacked > 0)
		{
			// But less than AIAttacked.
			if (Block->HumanAttacked <= Block->AIAttacked)
			{
				// If this AI is within can attack, go to that block.
				if (Block->IsNextToAffiliation(EAffiliation::HUMAN))
				{
					return Block;
				}

				DesirableBlocks.Add(Block);
			}
		}
	}

	// If you can find a HumanAttacked > 0, but it is NOT less than AIAttacked, wait for reinforcements and stay out of range.
	if (DesirableBlocks.Num() == 0)
	{
		// Choose whether to band with other AIs, or go towards humans.

		int32 Evaluation = EvaluateMap();

		// If the difference in warriors is 2 either way:
		if (Evaluation < 2 && Evaluation > -2)
		{
			// If still quite healthy, alight to target Humans.
			if (Health > 14)
			{
				return MoveTowardsConcentrationOfHumans();
			}

			// Otherwise, cohesion with AI.
			return MoveTowardsConcentrationOfAI();
		}
		
		// If AI has 3 more warriors than Human:
		if (Evaluation >= 3)
		{
			// Align to nearest Human.
			return FindNearestAffiliation(EAffiliation::HUMAN);
		}
		else
		{
			// Cohesion with AI.
			return FindNearestAffiliation(EAffiliation::AI);
		}
	}

	CurrentPath.Empty();

	// Find a block that minimises the distance to an enemy.
	ABlock* BestMove = CurrentBlock->GetClosestBlockToAHuman(DesirableBlocks);
	return BestMove;
}

ABlock* AWarrior::MoveTowardsConcentrationOfHumans()
{
	if (!UMapMaker::HumanConcentration)
	{
		UMapMaker::GenerateLargestConcentrationOfHumans();
	}

	ABlock* HumanHeatmap = UMapMaker::HumanConcentration;

	return MoveTowardsBlock(HumanHeatmap);
}

ABlock* AWarrior::MoveTowardsConcentrationOfAI()
{
	if (!UMapMaker::AIConcentration)
	{
		UMapMaker::GenerateLargestConcentrationOfAI();
	}

	ABlock* AIHeatmap = UMapMaker::AIConcentration;

	return MoveTowardsBlock(AIHeatmap);
}

ABlock* AWarrior::FindNearestAffiliation(const EAffiliation& Nearest)
{
	if (NumberOfHuman <= 0)
	{
		UMW::Log("NUMBER OF HUMANS IS ZERO");
		return CurrentBlock;
	}

	if (NumberOfAI <= 1) {
		UMW::Log("NUMBER OF AI IS ZERO");
		return CurrentBlock;
	}

	TSet<ABlock*> Visited;
	TQueue<ABlock*> Breadth;
	Breadth.Enqueue(CurrentBlock);
	ABlock* NextToAffiliation = CurrentBlock;

	while (!Breadth.IsEmpty())
	{
		ABlock* FrontBlock = *Breadth.Peek();
		Breadth.Pop();

		for (int i = 0; i < 8; ++i)
		{
			ABlock* QueryBlock = FrontBlock->Get(i);

			if (QueryBlock)
			{
				if (QueryBlock->IsNextToAffiliation(Nearest))
				{
					NextToAffiliation = QueryBlock;
					Breadth.Empty();
					break;
				}

				if (!Visited.Contains(QueryBlock))
				{
					Visited.Add(QueryBlock);
					Breadth.Enqueue(QueryBlock);
				}
			}
		}
	}

	return MoveTowardsBlock(NextToAffiliation);
}

void AWarrior::DealDamage()
{
	TArray<AWarrior*> SurroundingWarriors = GetAttackableWarriors();

	for (AWarrior* Warrior : SurroundingWarriors)
	{
		Warrior->DeductHealth();
	}

	Health -= SurroundingWarriors.Num() * .5f;
}

void AWarrior::MoveTowards()
{
	UE_LOG(LogTemp, Warning, TEXT("MOVING"));
	SetActorLocation(DirectionToPath);
}

