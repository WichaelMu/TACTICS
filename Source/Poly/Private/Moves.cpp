// Fill out your copyright notice in the Description page of Project Settings.


#include "Moves.h"

UMoves* UMoves::MakeMove(AWarrior* WarriorToMove, ABlock* BlockToMoveTo)
{
	UMoves* Move = nullptr;
	Move->Warrior = WarriorToMove;
	Move->Block = BlockToMoveTo;

	if (!Move)
	{
		UE_LOG(LogTemp, Warning, TEXT("MOVE IS NULL"));
	}

	return Move;
}
