// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapGenerator.generated.h"

class UMapMaker;
class UWarriorSpawner;

UCLASS()
class POLY_API AMapGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMapGenerator();

public:

	static AMapGenerator* Instance;

	void Tick(float DeltaTime) override;

	virtual bool ShouldTickIfViewportsOnly() const override;

	UPROPERTY(EditAnywhere)
		bool bGenerateMap;

	void InitialiseGame();
	UFUNCTION(Server, Reliable)
		void ServerInitialiseGame();

public:

	UPROPERTY(VisibleAnywhere)
		UMapMaker* MapMaker;
	UPROPERTY(VisibleAnywhere)
		UWarriorSpawner* WarriorSpawner;

};


AMapGenerator* AMapGenerator::Instance = nullptr;
