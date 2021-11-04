// Fill out your copyright notice in the Description page of Project Settings.


#include "WarriorHealthWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "MW.h"
#include "Net/UnrealNetwork.h"


UWarriorHealthWidget::UWarriorHealthWidget()
{
	SetWidgetSpace(EWidgetSpace::Screen);

	static ConstructorHelpers::FClassFinder<UUserWidget> UIHealthWidget(TEXT("/Game/UI/UI_Health"));
	if (UIHealthWidget.Succeeded())
	{
		SetWidgetClass(UIHealthWidget.Class);

		UserWidget = CreateWidget<UUserWidget>(GetWorld(), UIHealthWidget.Class);

		if (UserWidget)
		{
			UTextBlock* TextBlock = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("HEALTH_TEXT")));

			if (TextBlock)
			{
				HealthText = TextBlock;
			}
			else
			{
				UMW::LogError("UWarriorHealthWidget::UWarriorHealthWidget No TextBlock");
			}
		}
		else
		{
			UMW::LogError("UWarriorHealthWidget::WarriorHealthWidget No User Widget");
		}
	}
	else
	{
		UMW::LogError("UWarriorHealthWidget::UWarriorHealthWidget Widget Unsuccessful");
	}

	SetDrawSize(FVector2D(50, 20));
}

void UWarriorHealthWidget::SetHealthText(const int& NewHealth)
{
	if (HealthText)
	{
		HealthText->SetText(FText::FromString("OMEGALUL"));
	}
	else
	{
		UMW::LogError("UWarriorHealthWidget::SetHealthText No HealthText");
	}
}

void UWarriorHealthWidget::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// ...

	DOREPLIFETIME(UWarriorHealthWidget, UserWidget);
	DOREPLIFETIME(UWarriorHealthWidget, HealthText);
}

