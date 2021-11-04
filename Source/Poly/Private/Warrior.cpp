// Fill out your copyright notice in the Description page of Project Settings.


#include "Warrior.h"
#include "Block.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "MapMaker.h"
#include "MouseController.h"
#include "DrawDebugHelpers.h"
#include "MW.h"
#include "Net/UnrealNetwork.h"
#include "MapGenerator.h"
#include "WarriorHealthWidget.h"


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

	Health = 20;

	UWarriorHealthWidget* TempHealthWidget = CreateDefaultSubobject<UWarriorHealthWidget>(TEXT("Warrior Health Component"));
	if (TempHealthWidget)
	{
		TempHealthWidget->SetIsReplicated(true);
		HealthWidget = TempHealthWidget;
		HealthWidget->AttachTo(Cube);
	}
	else
	{
		UMW::LogError("AWarrior::AWarrior No TempHealthWidget");
	}

	SetHealthText(Health);
}


// Called when the game starts or when spawned
void AWarrior::BeginPlay()
{
	Super::BeginPlay();
	

	SetHealthText(Health);

	AssignAffiliationColours();
	SetOwner(AMapGenerator::Instance);
}


// Called every frame
void AWarrior::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AWarrior::ShouldTickIfViewportsOnly() const
{
	return true;
}


// Positive = AI advantage. Negative = Human advantage. 0 = Equal material.
int32 AWarrior::EvaluateMap()
{
	return NumberOfAI - NumberOfHuman;
}

void AWarrior::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// ...

	DOREPLIFETIME(AWarrior, Health);
	DOREPLIFETIME(AWarrior, CurrentBlock);
	DOREPLIFETIME(AWarrior, PreviousBlock);
	DOREPLIFETIME(AWarrior, Affiliation);
	DOREPLIFETIME(AWarrior, HealthWidget);
}


// When this warrior is spawned into the world.
void AWarrior::OnSpawn(ABlock* SpawnedBlock, EAffiliation TeamAffiliation)
{
	ServerOnSpawn(SpawnedBlock, TeamAffiliation);
}

void AWarrior::ServerOnSpawn_Implementation(ABlock* SpawnedBlock, EAffiliation TeamAffiliation)
{
	Health = 20;

	this->Affiliation = TeamAffiliation;
	UMW::Log("AWarrior::OnSpawn Warrior Spawned");
	if (SpawnedBlock)
	{
		// Default Previous and Current Block to the block this Warrior spawned on.
		PreviousBlock = SpawnedBlock;
		CurrentBlock = SpawnedBlock;
		SpawnedBlock->Occupant = this;

		// Change the location of this warrior.
		SetActorLocation(SpawnedBlock->GetWarriorPosition());
		UMW::Log("AWarrior::OnSpawn Updated");
		UpdateBlock(SpawnedBlock);

		// Defined in blueprint.
		AssignAffiliationColours();

		SetHealthText(Health);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("NO SPAWNED BLOCK."));
	}

	SpawnedBlock->Selected(true);
}


// Move this warrior to TargetBlock.
void AWarrior::MoveTo(ABlock* TargetBlock)
{
	UMW::Log("Is Being Called");
	UMW::Log("Warrior count: " + FString::SanitizeFloat(NumberOfHuman));
	switch (GetLocalRole())
	{
	case ROLE_Authority:
		UMW::Log("AWarrior::AUTH");
		break;
	case ROLE_AutonomousProxy:
		UMW::Log("AWarrior::PROX");
		break;
	case ROLE_SimulatedProxy:
		UMW::Log("AWarrior::SIMU");
		break;
	case ROLE_None:
		UMW::Log("AWarrior::NONE");
		break;
	case ROLE_MAX:
		UMW::Log("AWarrior::MAX");
		break;
	default:
		UMW::Log("AWarrior::WHAT");
	}

	if (TargetBlock)
	{
		ServerMoveTo(TargetBlock);
		RegisterMovement(TargetBlock);
	}
	else
	{
		UMW::LogError("AWarrior::MoveTo No TargetBlock");
	}
}


void AWarrior::ServerMoveTo_Implementation(ABlock* TargetBlock)
{
	UMW::Log("Move To");
	// TODO: Change the location of the warrior with interpolation and pathfinding.
	if (CurrentBlock)
	{
		if (CurrentBlock != TargetBlock)
		{
			SetActorLocation(TargetBlock->GetWarriorPosition());
		}

		UpdateBlock(TargetBlock);
	}

	DealDamage();
}

// Update block information when moving between blocks.
void AWarrior::UpdateBlock(ABlock* NewBlock)
{
	ServerUpdateBlock(NewBlock);
}


void AWarrior::ServerUpdateBlock_Implementation(ABlock* NewBlock)
{
	PreviousBlock = CurrentBlock;

	CurrentBlock->Occupant = nullptr;

	UpdateBlockAttacks(CurrentBlock, NewBlock);

	NewBlock->Occupant = this;
	CurrentBlock = NewBlock;
}

// Update the attack references on the block.
void AWarrior::UpdateBlockAttacks(ABlock* From, ABlock* To)
{
	// Prevent deducting attacks On Spawn.
	if (From != To)
	{
		From->DeductAttacks(Affiliation);
	}

	To->AppendAttacks(Affiliation);
}


// Move towards Relative, accounting for traversable blocks and pathfinding.
ABlock* AWarrior::MoveTowardsBlock(ABlock* Relative)
{
	//CurrentPath = UMW::Pathfind(CurrentBlock, Relative);
	CurrentPath = TNavigator<ABlock>::Pathfind(CurrentBlock, Relative, UMapMaker::Instance->XMap * UMapMaker::Instance->YMap);

	for (int i = 0; i < CurrentPath.Num() - 1; ++i)
	{
		DrawDebugLine(GetWorld(), CurrentPath[i]->GetWarriorPosition(), CurrentPath[i + 1]->GetWarriorPosition(), FColor::Blue, true, 10, 0, 10);
	}

	// Go towards the path if Relative is too far away.
	if (CurrentPath.Num() > 1)
	{
		return CurrentPath.Last(1);
	}

	// Below is distance-based pathfinding in case UMW::Pathfind(ABlock*, ABlock*) fails.
	// It usually fails if the target is exactly 1 block away from Relative, in which case,
	// it is completely fine to use distance to determine a path, as it is already close enough.

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


void AWarrior::SetHealthText(const int& NewHealth)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (HealthWidget)
		{
			HealthWidget->SetHealthText(NewHealth);
		}
		else
		{
			UMW::LogError("AWarrior::SetHealthText No Health Widget");
		}
	}
}

// Revive some health.
int AWarrior::Revive()
{
	int NewHealth = FMath::Min<int>(Health + 1, 20);
	Health = NewHealth;
	SetHealthText(Health);

	return NewHealth;
}


// Take damage.
void AWarrior::DeductHealth()
{
	Health -= Damage;
	SetHealthText(Health);

	if (HealthIsFatal())
	{
		KillThisWarrior();
	}
}


// Has the health fallen below zero?
bool AWarrior::HealthIsFatal()
{
	return Health <= 0;
}


// Disassociate this warrior as part of Poly.
void AWarrior::KillThisWarrior()
{
	ServerKillThisWarrior();
}

void AWarrior::ServerKillThisWarrior_Implementation()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		// Deregister this Warrior as part of all warriors.
		UMapMaker::Instance->AllWarriors.Remove(this);

		// Move this warrior somewhere out of the screen.
		SetActorLocation(FVector(40000.f));

		// Deregister the Current Block's attacks and occupant.
		CurrentBlock->DeductAttacks(Affiliation);
		CurrentBlock->Occupant = nullptr;
		CurrentBlock = nullptr;

		// Reduce the number of Affiliation members.
		if (Affiliation == EAffiliation::HUMAN1 || Affiliation == EAffiliation::HUMAN2)
		{
			NumberOfHuman--;
		}
		else
		{
			NumberOfAI--;
		}
	}
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
	MoveTo(MoveTowardsBlock((ConcentrationOfHumans())));
}


ABlock* AWarrior::ConcentrationOfHumans()
{
	if (!UMapMaker::HumanConcentration)
	{
		UMapMaker::GenerateLargestConcentrationOfHumans();
	}

	ABlock* HumanHeatmap = UMapMaker::HumanConcentration;

	return HumanHeatmap;
}


// Retreat.


ABlock* AWarrior::FindSafestBlock()
{
	TArray<AWarrior*> NearbyEnemies = CurrentBlock->SurroundingEnemiesInRange(Affiliation);

	// If there are no enemies in the range of 1 ply, or if the health is critical, Revive.
	if (NearbyEnemies.Num() == 0 && Health <= 4)
	{
		Revive();
		return CurrentBlock;
	}

	TArray<ABlock*> Traversable = CurrentBlock->GetTraversableBlocks();

	// Default values for safest block.
	ABlock* MaximumAI = CurrentBlock;
	ABlock* MinimalHumans = CurrentBlock;

	int MaxAI = INT_MIN;
	int MinHuman = INT_MAX;

	// For every block that is traversable, find the number of AI/Human influence on them.
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

	// Prioritise going towards AI if there are more of them, otherwise, go towards the place with minimal humans.
	return MaxAI > MinHuman ? MaximumAI : MinimalHumans;
}


// Attack.


ABlock* AWarrior::FindKillableHuman()
{
	// AI has already won.
	if (NumberOfHuman == 0)
	{
		UMW::Log("AI HAS WON.");
		ABlock* Winning = ConcentrationOfHumans();

		if (Winning)
		{
			return MoveTowardsBlock(Winning); // Will default to centre.
		}
		else
		{
			UMW::LogError("AWarrior::FindKillableHuman No Winning");
			return CurrentBlock;
		}
	}

	// If this block is overwhelmed by humans, run away || stay out of range.
	if (CurrentBlock->AIAttacked < CurrentBlock->HumanAttacked)
	{
		ABlock* Safest = FindSafestBlock();

		if (Safest)
		{
			return Safest;
		}
		else
		{
			UMW::LogError("AWarrior::FindKillableHuman No Safest");
		}
	}

	TArray<ABlock*> Traversable = CurrentBlock->GetTraversableBlocks();

	uint32 DesirableBlocks = 0;

	for (ABlock* Block : Traversable)
	{
		// Find a block where HumanAttacked > 0:
		if (Block->HumanAttacked > 0)
		{
			// But less than AIAttacked.
			if (Block->HumanAttacked <= Block->AIAttacked)
			{
				// If this AI is within can attack, go to that block.
				if (Block->IsNextToAffiliation(EAffiliation::HUMAN1) || Block->IsNextToAffiliation(EAffiliation::HUMAN2))
				{
					return Block;
				}

				DesirableBlocks++;
			}
		}
	}

	// If you can find a HumanAttacked > 0, but it is NOT less than AIAttacked, wait for reinforcements and stay out of range.
	if (DesirableBlocks == 0)
	{
		// Choose whether to band with other AIs, or go towards humans.

		int32 Evaluation = EvaluateMap();

		// If the difference in warriors is 2 either way:
		if (Evaluation < 2 && Evaluation > -2)
		{
			// If still quite healthy, align to target Humans.
			if (Health > 16)
			{
				ABlock* NearestHuman = FindNearestHumanAffiliation(EAffiliation::HUMAN1);

				if (NearestHuman)
				{
					// If the block-distance to NearestHuman is greater than 6. Flank that position.
					if (TNavigator<ABlock>::Pathfind(CurrentBlock, NearestHuman, UMapMaker::Instance->XMap * UMapMaker::Instance->YMap).Num() > 6)
					{
						return MoveTowardsBlock(Flank(NearestHuman));
					}

					// Go towards NearestHuman.
					return MoveTowardsBlock(NearestHuman);
				}
				else
				{
					UMW::LogError("AWarrior::FindKillableHuman No NearestHuman");
					return CurrentBlock;
				}
			}

			// Otherwise, cohesion with group of AI.
			ABlock* Cohesion = ConcentrationOfAI();

			if (Cohesion)
			{
				return MoveTowardsBlock(Cohesion);
			}
			else
			{
				UMW::LogError("AWarrior::FindKillableHuman Desirable but No Cohesion");
			}
		}

		// If AI has 2 more warriors than Human:
		if (Evaluation >= 2)
		{
			// Align to nearest Human by prediction.
			ABlock* Alignment = Flank(FindNearestHumanAffiliation(EAffiliation::HUMAN1));

			if (Alignment)
			{
				return MoveTowardsBlock(Alignment);
			}
			else
			{
				UMW::LogError("AWarrior::FindKillableHuman No Alignment");
				return CurrentBlock;
			}
		}
		else
		{
			// Cohesion with closest AI.

			ABlock* Cohesion = FindNearestHumanAffiliation(EAffiliation::AI);
			if (Cohesion)
			{
				return MoveTowardsBlock(Cohesion);
			}
			else
			{
				UMW::LogError("AWarrior::FindKillableHuman No Cohesion");
			}
		}
	}

	CurrentPath.Empty();

	// Find a block that minimises the distance to an enemy.
	ABlock* BestMove = MoveTowardsBlock(FindNearestHumanAffiliation(EAffiliation::HUMAN1));
	if (BestMove)
	{
		return BestMove;
	}
	else
	{
		UMW::LogError("AWarrior::FindKillableHuman No Default.");
		return CurrentBlock;
	}
}


// Flanks position.
ABlock* AWarrior::Flank(ABlock* Position)
{
	if (Position == CurrentBlock)
	{
		// Couldn't find the closest Human, nothing we can do. Return the same block.
		// This should only ever happen if AI has won, in which case, this won't be called.
		return Position;
	}

	TArray<ABlock*> FlankRoute = ABlock::ComputeTrajectory(Position, UMapMaker::Instance->XMap);

	// The depth or terrain limited length of the trajectory.
	int32 FlankRouteLength = FlankRoute.Num();

	// The block-distance between the current AI and the nearest Human.
	int32 DistanceToTarget = TNavigator<ABlock>::Pathfind(CurrentBlock, Position, UMapMaker::Instance->XMap * UMapMaker::Instance->YMap).Num();

	if (DistanceToTarget > FlankRouteLength)
	{
		// The target is too far, go towards the end of the trajectory.
		return FlankRoute.Last();
	}

	// Half distance is the midpoint for reaching the predicted trajectory.
	int32 Time = DistanceToTarget / 2;

	if (Time <= DistanceToTarget)
	{
		if (FlankRoute.IsValidIndex(Time))
		{
			// Close in on half distance.
			return FlankRoute[Time];
		}
	}

	// Default. Return the last element in the trajectory.
	return FlankRoute.Last();
}


ABlock* AWarrior::ConcentrationOfAI()
{
	if (!UMapMaker::AIConcentration)
	{
		UMapMaker::GenerateLargestConcentrationOfAI();
	}

	ABlock* AIHeatmap = UMapMaker::AIConcentration;

	return AIHeatmap;
}

// Conducts a BFS to search for Nearest.
ABlock* AWarrior::FindNearestHumanAffiliation(const EAffiliation& Nearest)
{
	// AI has already won. Do not continue.
	if (NumberOfHuman <= 0)
	{
		UMW::Log("NUMBER OF HUMANS IS ZERO");
		return CurrentBlock;
	}

	// AI has lost. Do not continue.
	if (NumberOfAI <= 0)
	{
		UMW::Log("NUMBER OF AI IS ZERO");
		return CurrentBlock;
	}

	TSet<ABlock*> Visited;
	TQueue<ABlock*> Breadth;
	Breadth.Enqueue(CurrentBlock);
	ABlock* NextToAffiliation = CurrentBlock;

	// Executes a BFS starting from CurrentBlock, fanning outwards until a Nearest warrior is found.
	while (!Breadth.IsEmpty())
	{
		ABlock* FrontBlock = *Breadth.Peek();
		Breadth.Pop();

		for (int i = 0; i < 8; ++i)
		{
			ABlock* QueryBlock = FrontBlock->Get(i);

			if (QueryBlock)
			{
				// The Nearest warrior has been found, exit.
				if (QueryBlock->IsNextToAffiliation(EAffiliation::HUMAN1) || QueryBlock->IsNextToAffiliation(EAffiliation::HUMAN2))
				{
					// The block that is next to or in striking range of Nearest.
					return QueryBlock;
				}

				// Do not waste time queueing something that has been visited.
				if (!Visited.Contains(QueryBlock))
				{
					Visited.Add(QueryBlock);
					Breadth.Enqueue(QueryBlock);
				}
			}
		}
	}

	// This is executed when BFS can't find a reachable Human.
	// Go towards the largest concentration of Nearest instead. (At least try to get to the enemy).
	return Nearest == EAffiliation::HUMAN1 || Nearest == EAffiliation::HUMAN2 ? ConcentrationOfHumans() : ConcentrationOfAI();
}


void AWarrior::DealDamage()
{
	TArray<AWarrior*> SurroundingWarriors = GetAttackableWarriors();

	// Deal damage to all opposition.
	for (AWarrior* Warrior : SurroundingWarriors)
	{
		Warrior->DeductHealth();
	}
}

// Gets every ABlock around CurrentBlock if it is occupied and the occupant is the opposition.
TArray<AWarrior*> AWarrior::GetAttackableWarriors()
{
	TArray<AWarrior*> Attackable;

	if (CurrentBlock)
	{
		for (int32 i = 0; i < 8; ++i)
		{
			ABlock* Neighbour = CurrentBlock->Get(i);
			if (Neighbour)
			{
				AWarrior* SurroundingOccupant = Neighbour->Occupant;
				if (SurroundingOccupant)
				{
					if (SurroundingOccupant->Affiliation != Affiliation)
					{
						Attackable.Add(SurroundingOccupant);
					}
				}
			}
		}
	}

	return Attackable;
}

