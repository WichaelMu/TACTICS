// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "WarriorHUD.generated.h"

/**
 * 
 */
UCLASS()
class POLY_API AWarriorHUD : public AHUD
{
	GENERATED_BODY()
	
public:

	AWarriorHUD();

public:

	void SetHealthDisplay(const int& NewHealth);

private:

	TSubclassOf<UUserWidget> HUDClass;
	UUserWidget* CurrentWarriorHUDWidget;

	UTextBlock* HealthText;
};
