// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Loadout/LoadoutWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"

void ULoadoutWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HeadTabButton)
	{
		HeadTabButton->OnClicked.AddDynamic(this, &ULoadoutWidget::HandleHeadTabClicked);
	}
	if (BodyTabButton)
	{
		BodyTabButton->OnClicked.AddDynamic(this, &ULoadoutWidget::HandleBodyTabClicked);
	}
	if (WeaponTabButton)
	{
		WeaponTabButton->OnClicked.AddDynamic(this, &ULoadoutWidget::HandleWeaponTabClicked);
	}

	SetActiveTab(0);
}

void ULoadoutWidget::HandleHeadTabClicked()
{
	SetActiveTab(0);
}

void ULoadoutWidget::HandleBodyTabClicked()
{
	SetActiveTab(1);
}

void ULoadoutWidget::HandleWeaponTabClicked()
{
	SetActiveTab(2);
}

void ULoadoutWidget::SetActiveTab(int32 TabIndex)
{
	if (TabSwitcher)
	{
		TabSwitcher->SetActiveWidgetIndex(TabIndex);
	}
}
