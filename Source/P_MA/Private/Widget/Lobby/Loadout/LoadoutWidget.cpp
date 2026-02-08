// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Level/Lobby/LobbyPlayerController.h"
#include "Widget/Lobby/Loadout/LoadoutBodyTabWidget.h"
#include "Widget/Lobby/Loadout/LoadoutHeadTabWidget.h"
#include "Widget/Lobby/Loadout/LoadoutWeaponTabWidget.h"

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

	SetActiveTab(1);
	if (ALobbyPlayerController* PC = GetOwningPlayer<ALobbyPlayerController>())
	{
		PC->SetLoadoutView(ALobbyPlayerController::ELoadoutView::Body);
	}
}

void ULoadoutWidget::HandleHeadTabClicked()
{
	SetActiveTab(0);
	if (ALobbyPlayerController* PC = GetOwningPlayer<ALobbyPlayerController>())
	{
		PC->SetLoadoutView(ALobbyPlayerController::ELoadoutView::Head);
	}
}

void ULoadoutWidget::HandleBodyTabClicked()
{
	SetActiveTab(1);
	if (ALobbyPlayerController* PC = GetOwningPlayer<ALobbyPlayerController>())
	{
		PC->SetLoadoutView(ALobbyPlayerController::ELoadoutView::Body);
	}
}

void ULoadoutWidget::HandleWeaponTabClicked()
{
	SetActiveTab(2);
	if (ALobbyPlayerController* PC = GetOwningPlayer<ALobbyPlayerController>())
	{
		PC->SetLoadoutView(ALobbyPlayerController::ELoadoutView::Weapon);
	}
}

void ULoadoutWidget::SetActiveTab(int32 TabIndex)
{
	if (TabSwitcher)
	{
		TabSwitcher->SetActiveWidgetIndex(TabIndex);
	}

	if (HeadTabButton)
	{
		HeadTabButton->SetIsEnabled(TabIndex != 0);
	}
	if (BodyTabButton)
	{
		BodyTabButton->SetIsEnabled(TabIndex != 1);
	}
	if (WeaponTabButton)
	{
		WeaponTabButton->SetIsEnabled(TabIndex != 2);
	}

}
