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

	static void RunAI();
	static void Log(FString Message);
	static void LogError(FString Message);

	static bool IsBlockTraversable(ABlock* Query);

	template <typename T>
	static T* Coalesce(T* Left, T* Right);

private:

	static void DetermineMoves();

};


template <typename T>
class TNavigator
{

public:

	static TArray<T*> Pathfind(T*, T*, uint16);

};


template <typename T> // Where T : THeapItem<T>, TNode<T>.
TArray<T*> TNavigator<T>::Pathfind(T* Origin, T* Destination, uint16 SizeOfMap)
{
	TArray<T*> Path;
	THeap<T> Open(Origin, SizeOfMap);
	TSet<T*> Closed;

	Open.Add(Origin);

	Origin->G = 0;
	Origin->H = FVector::DistSquared(Origin->GetActorLocation(), Destination->GetActorLocation());

	while (Open.Count > 0)
	{
		T* Current = Open.RemoveFirst();
		Closed.Add(Current);

		if (Current == Destination)
		{
			T* Traverse = Destination;
			while (Traverse != Origin)
			{
				Path.Add(Traverse);
				Traverse = Traverse->Parent;
			}

			return Path;
		}

		for (int i = 0; i < 8; ++i)
		{
			T* Query = Current->Get(i);
			if (Query)
			{
				if (!UMW::IsBlockTraversable(Query) || Closed.Contains(Query))
				{
					continue;
				}

				float FUpdatedCost = Current->G + FVector::DistSquared(Current->GetActorLocation(), Query->GetActorLocation());
				if (FUpdatedCost < Query->G || !Open.Contains(Query))
				{
					Query->G = FUpdatedCost;
					Query->H = FVector::DistSquared(Query->GetActorLocation(), Destination->GetActorLocation());
					Query->Parent = Current;

					if (!Open.Contains(Query))
					{
						Open.Add(Query);
					}
					else
					{
						Open.UpdateItem(Query);
					}
				}
			}
		}
	}

	return Path;
}


/*
* A Heap used for minimum sorting to find candidate Node with lowest Node::F in UMW::Pathfinding.
*/
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
	void UpdateItem(T* Item);
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


// Adds Item to this Heap.
template <typename T>
void THeap<T>::Add(T* Item)
{
	Item->HeapItemIndex = Count;
	Items[Count] = Item;

	SortUp(Item);

	Count++;
}


// Returns the root and removes it.
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


// Update the Heap after Item has changed.
template <typename T>
void THeap<T>::UpdateItem(T* Item)
{
	SortUp(Item);
	SortDown(Item);
}


// Does Item exist in this Heap?
template <typename T>
bool THeap<T>::Contains(const T* Item) const
{
	return Items[Item->HeapItemIndex] == Item;
}


// Sort from Item upwards.
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


// Sort from Item downwards.
template <typename T>
void THeap<T>::SortDown(T* Item)
{
	while (1)
	{
		int32 Left = Item->HeapItemIndex * 2 + 1;	// L and R Children.
		int32 Right = Item->HeapItemIndex * 2 + 2;	//
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


/*
* THeap<T> where T : THeapItem<T>.
*/
template <typename T>
class THeapItem
{

public:

	int32 HeapItemIndex;
	virtual int CompareTo(T* Block) { return 0; }
	virtual T* Get(const uint8& Orientation) const { return nullptr; }

};


template <typename T>
class TNode
{

public:

	T* Parent;
	float F, G, H;
	virtual FVector GetWorldPosition() { return FVector::ZeroVector; }
};

