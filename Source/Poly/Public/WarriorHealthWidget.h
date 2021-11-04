// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "Components/TextBlock.h"
#include "WarriorHealthWidget.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class POLY_API UWarriorHealthWidget : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	UWarriorHealthWidget();

public:

	void SetHealthText(const int& NewHealth);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, VisibleAnywhere)
		UUserWidget* UserWidget;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
		UTextBlock* HealthText;
};
