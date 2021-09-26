// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MW.generated.h"

class AWarrior;
class ABlock;
class UMoves;

/**
 * Pathfinding, AI and other helper functions for Poly.
 */
UCLASS()
class POLY_API UMW : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static bool Pathfind(ABlock* Origin, ABlock* Destination, TArray<ABlock*>& Path);
	static TArray<ABlock*> Pathfind(ABlock* Origin, ABlock* Destination);

	static void RunAI();
	static void Log(FString Message);

	static bool IsBlockTraversable(ABlock* Query);

private:

	static void DetermineMoves();

};


template <typename T>
class THeap
{

public:

	THeap(T* Default, int32 Size);
	~THeap();

	TArray<T*> Items;
	int32 Count;

	void Add(T* Item);
	T* RemoveFirst();
	bool Contains(const T* Item) const;

private:

	void SortDown(T* Item);
	void SortUp(T* Item);
	void Swap(T* Left, T* Right);

};


template <typename T>
THeap<T>::THeap(T* Default, int32 Size)
{
	Items.Init(Default, Size);
	
	Count = 0;
}


template <typename T>
THeap<T>::~THeap()
{
	Items.Empty();
}


template <typename T>
void THeap<T>::Add(T* Item)
{
	Item->HeapItemIndex = Count;
	Items[Count] = Item;

	SortUp(Item);

	Count++;
}


template <typename T>
T* THeap<T>::RemoveFirst()
{
	T* First = Items[0];
	Count--;

	Items[0] = Items[Count];
	Items[0]->HeapItemIndex = 0;
	
	SortDown(Items[0]);

	return First;
}

template <typename T>
bool THeap<T>::Contains(const T* Item) const
{
	return Items[Item->HeapItemIndex] == Item;
}

template <typename T>
void THeap<T>::SortUp(T* Item)
{
	int32 Parent = (Item->HeapItemIndex - 1) / 2;

	while (1)
	{
		T* ParentItem = Items[Parent];

		if (Item->CompareTo(ParentItem) > 0)
		{
			Swap(Item, ParentItem);
		}
		else
		{
			break;
		}

		Parent = (Item->HeapItemIndex - 1) / 2;
	}
}

template <typename T>
void THeap<T>::SortDown(T* Item)
{
	while (1)
	{
		int32 Left = Item->HeapItemIndex * 2 + 1;
		int32 Right = Item->HeapItemIndex * 2 + 2;
		uint32 SwapIndex = 0;

		if (Left < Count)
		{
			SwapIndex = Left;

			if (Right < Count)
			{
				if (Items[Left]->CompareTo(Items[Right]) < 0)
				{
					SwapIndex = Right;
				}
			}

			if (Item->CompareTo(Items[SwapIndex]) < 0)
			{
				Swap(Item, Items[SwapIndex]);
			}
			else
			{
				return;
			}
		}
		else
		{
			return;
		}
	}
}

template <typename T>
void THeap<T>::Swap(T* Left, T* Right)
{
	Items[Left->HeapItemIndex] = Right;
	Items[Right->HeapItemIndex] = Left;

	int32 Temp = Left->HeapItemIndex;
	Left->HeapItemIndex = Right->HeapItemIndex;
	Right->HeapItemIndex = Temp;
}


template <typename T>
class THeapItem
{

public:

	int32 HeapItemIndex;
	virtual int CompareTo(T*) { return 0; }
	virtual T* Get(const uint8& Orientation) const { return nullptr; }

};

