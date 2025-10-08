// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/MAMobilityChargeWidget.h"
#include "Player/MAPlayerCharacter.h" 
#include "Components/ProgressBar.h"

void UMAMobilityChargeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	HideChargeBar();

	AMAPlayerCharacter* Character = Cast<AMAPlayerCharacter>(GetOwningPlayerPawn());
	if (Character)
	{
		// 캐릭터의 델리게이트와 C++ 함수들을 연결
		Character->OnChargeAbilityStarted.AddDynamic(this, &UMAMobilityChargeWidget::ShowChargeBar);
		Character->OnChargeAbilityUpdate.AddDynamic(this, &UMAMobilityChargeWidget::UpdateChargeBar);
		Character->OnChargeAbilityEnded.AddDynamic(this, &UMAMobilityChargeWidget::HideChargeBar);
	}
}

void UMAMobilityChargeWidget::ShowChargeBar()
{
	if (ChargeProgressBar)
	{
		ChargeProgressBar->SetVisibility(ESlateVisibility::Visible);
		ChargeProgressBar->SetPercent(0.0f);
	}
}

void UMAMobilityChargeWidget::UpdateChargeBar(float ChargePercentage)
{
	if (ChargeProgressBar)
	{
		ChargeProgressBar->SetPercent(ChargePercentage);
	}
}

void UMAMobilityChargeWidget::HideChargeBar()
{
	if (ChargeProgressBar)
	{
		ChargeProgressBar->SetVisibility(ESlateVisibility::Hidden);
	}
}
