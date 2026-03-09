// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Level/Lobby/LobbyPlayerController.h"
#include "Widget/Lobby/Loadout/LoadoutBodyTabWidget.h"
#include "Widget/Lobby/Loadout/LoadoutHeadTabWidget.h"
#include "Widget/Lobby/Loadout/LoadoutMountTabWidget.h"
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
	if (MountTabButton)
	{
		MountTabButton->OnClicked.AddDynamic(this, &ULoadoutWidget::HandleMountTabClicked);
	}

	SetActiveTab(1);
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
		PC->ApplyPendingWeaponPreviewDelayed(0.3f);
	}
}

void ULoadoutWidget::HandleMountTabClicked()
{
	SetActiveTab(3);
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

void ULoadoutWidget::SyncSelectionFromPending(const FMaterialParamDataPair& PendingColor, FName PendingEyeShapeId, FName PendingWeaponId, FName PendingMountId)
{
	if (BodyTabWidget)
	{
		BodyTabWidget->SyncFromPendingBody(PendingColor.BodyData);
	}

	if (HeadTabWidget)
	{
		HeadTabWidget->SyncFromPendingHead(PendingColor.EyeData, PendingEyeShapeId);
	}

	if (WeaponTabWidget)
	{
		WeaponTabWidget->SyncFromPendingWeapon(PendingWeaponId);
	}

	if (MountTabWidget)
	{
		MountTabWidget->SyncFromPendingMount(PendingMountId);
	}
}
