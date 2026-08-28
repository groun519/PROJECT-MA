// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Widget/Lobby/Loadout/LoadoutBodyTabWidget.h"
#include "Widget/Lobby/Loadout/LoadoutHeadTabWidget.h"
#include "Widget/Lobby/Loadout/LoadoutMountTabWidget.h"
#include "Widget/Lobby/Loadout/LoadoutWeaponTabWidget.h"

void ULoadoutWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HeadTabButton)
	{
		HeadTabButton->OnClicked.AddUniqueDynamic(this, &ULoadoutWidget::HandleHeadTabClicked);
	}
	if (BodyTabButton)
	{
		BodyTabButton->OnClicked.AddUniqueDynamic(this, &ULoadoutWidget::HandleBodyTabClicked);
	}
	if (WeaponTabButton)
	{
		WeaponTabButton->OnClicked.AddUniqueDynamic(this, &ULoadoutWidget::HandleWeaponTabClicked);
	}
	if (MountTabButton)
	{
		MountTabButton->OnClicked.AddUniqueDynamic(this, &ULoadoutWidget::HandleMountTabClicked);
	}

	SetActiveTab(1);
}

void ULoadoutWidget::HandleHeadTabClicked()
{
	SetActiveTab(0);
	TabSelectedDelegate.Broadcast(ELoadoutTab::Head);
}

void ULoadoutWidget::HandleBodyTabClicked()
{
	SetActiveTab(1);
	TabSelectedDelegate.Broadcast(ELoadoutTab::Body);
}

void ULoadoutWidget::HandleWeaponTabClicked()
{
	SetActiveTab(2);
	TabSelectedDelegate.Broadcast(ELoadoutTab::Weapon);
}

void ULoadoutWidget::HandleMountTabClicked()
{
	SetActiveTab(3);
	TabSelectedDelegate.Broadcast(ELoadoutTab::Mount);
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
	if (MountTabButton)
	{
		MountTabButton->SetIsEnabled(TabIndex != 3);
	}

}

void ULoadoutWidget::ActivateBodyTabUI()
{
	SetActiveTab(1);
}

void ULoadoutWidget::SyncSelectionFromPending(const FLoadoutSelection& PendingLoadout)
{
	if (BodyTabWidget)
	{
		BodyTabWidget->SyncFromPendingBody(PendingLoadout.Color.BodyData);
	}

	if (HeadTabWidget)
	{
		HeadTabWidget->SyncFromPendingHead(PendingLoadout.Color.EyeData, PendingLoadout.EyeShapeId);
	}

	if (WeaponTabWidget)
	{
		WeaponTabWidget->SyncFromPendingWeapon(PendingLoadout.WeaponId);
	}

	if (MountTabWidget)
	{
		MountTabWidget->SyncFromPendingMount(PendingLoadout.MountId);
	}
}
