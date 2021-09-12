// Fill out your copyright notice in the Description page of Project Settings.


#include "Warrior.h"
#include "Block.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "MapMaker.h"
#include "DrawDebugHelpers.h"
#include "MW.h"
#include "TimerManager.h"

// Sets default values
AWarrior::AWarrior()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;


	const FVector WarriorScale = FVector(.5f, .5f, 1.5f);

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
			//		DrawDebugLine(GetWorld(), Path[i]->GetWarriorPosition(), Path[i - 1]->GetWarriorPosition(), FColor::White, true, 10, 0U, 10);
			//	}
			//}

			//SetActorLocation(Path[0]->GetWarriorPosition());

			//Path.Empty();
			
			SetActorLocation(TargetBlock->GetWarriorPosition());
		}
	}

	UpdateBlock(TargetBlock);
}

// Update block information when moving between blocks.
void AWarrior::UpdateBlock(ABlock* NewBlock)
{
	CurrentBlock->Occupant = nullptr;

	UpdateBlockAttacks(CurrentBlock, NewBlock);

	NewBlock->Occupant = this;
	CurrentBlock = NewBlock;
}

void AWarrior::UpdateBlockAttacks(ABlock* From, ABlock* To)
{
	TArray<ABlock*> Before = From->SearchAtDepth(3);
	TArray<ABlock*> After = To->SearchAtDepth(3);

	bool bIsHuman = Affiliation == EAffiliation::HUMAN;

	if (From != To)
	{
		for (ABlock* Previous : Before)
		{
			if (bIsHuman)
			{
				Previous->HumanAttacked--;
			}
			else
			{
				Previous->AIAttacked--;
			}
		}
	}

	for (ABlock* Future : After)
	{
		if (bIsHuman)
		{
			Future->HumanAttacked++;
		}
		else
		{
			Future->AIAttacked++;
		}
	}
}

int AWarrior::Revive()
{
	int NewHealth = FMath::Max<int>(Health + 3, 20);
	Health = NewHealth;
	return NewHealth;
}

// Gets every ABlock around CurrentBlock if it is occupied.
TArray<ABlock*> AWarrior::GetAttackablePositions()
{
	TArray<ABlock*> Attackable;

	if (CurrentBlock)
	{
		for (int32 i = 0; i < 8; ++i)
		{
			if (CurrentBlock->Get(i)->Occupant)
			{
				Attackable.Add(CurrentBlock->Get(i));
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
	// If this block is overwhelmed by humans, run away.
	if (CurrentBlock->AIAttacked < CurrentBlock->HumanAttacked)
	{
		return FindSafestBlock();
	}

	TArray<ABlock*> Traversable = CurrentBlock->GetTraversableBlocks();

	TArray<ABlock*> DesirableBlocks;

	for (ABlock* Block : Traversable)
	{
		if (Block->HumanAttacked > 0)
		{
			// Find a block where HumanAttacked > 0, but less than AIAttacked.
			if (Block->HumanAttacked <= Block->AIAttacked)
			{
				if (Block->IsNextToHuman())
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
		return FindSafestBlock();
	}

	// Find a block that minimises the distance to an enemy.
	ABlock* BestMove = CurrentBlock->GetClosestBlockToAHuman(DesirableBlocks);

	return BestMove;
}

void AWarrior::MoveTowards()
{
	UE_LOG(LogTemp, Warning, TEXT("MOVING"));
	SetActorLocation(DirectionToPath);
}

