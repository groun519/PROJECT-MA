// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutWeaponTabWidget.h"
#include "Components/ScrollBox.h"
#include "Engine/DataTable.h"
#include "Framework/MAGameInstance.h"
#include "Player/Loadout/Data/LoadoutDataSet.h"
#include "Player/Loadout/Data/LoadoutWeaponData.h"
#include "Widget/Lobby/Loadout/LoadoutWeaponIconButtonWidget.h"
#include "Level/Lobby/LobbyPlayerController.h"

void ULoadoutWeaponTabWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildWeaponButtons();
}

void ULoadoutWeaponTabWidget::BuildWeaponButtons()
{
	const UMAGameInstance* GI = GetGameInstance<UMAGameInstance>();
	const ULoadoutDataSet* LoadoutDataSet = GI ? GI->TryGetLoadoutDataSet() : nullptr;
	const UDataTable* ResolvedWeaponDataTable = nullptr;
	if (LoadoutDataSet && LoadoutDataSet->WeaponDataTable)
	{
		ResolvedWeaponDataTable = LoadoutDataSet->WeaponDataTable;
	}

	if (!WeaponScrollBox || !WeaponButtonClass || !ResolvedWeaponDataTable)
	{
		return;
	}

	WeaponScrollBox->ClearChildren();
	WeaponButtons.Reset();

	TArray<FName> RowNames = ResolvedWeaponDataTable->GetRowNames();

	for (const FName RowName : RowNames)
	{
		const FLoadoutWeaponDataRow* Row = ResolvedWeaponDataTable->FindRow<FLoadoutWeaponDataRow>(RowName, TEXT("LoadoutWeaponTab"));
		if (!Row)
		{
			continue;
		}

		ULoadoutWeaponIconButtonWidget* ButtonWidget = CreateWidget<ULoadoutWeaponIconButtonWidget>(this, WeaponButtonClass);
		if (!ButtonWidget)
		{
			continue;
		}

		ButtonWidget->WeaponId = RowName;
		ButtonWidget->IconTexture = Row->IconTexture;
		ButtonWidget->OnWeaponSelected.AddDynamic(this, &ULoadoutWeaponTabWidget::HandleWeaponSelected);
		AddButtonToScrollBox(WeaponScrollBox, ButtonWidget);

		WeaponButtons.Add(ButtonWidget);
	}
}

void ULoadoutWeaponTabWidget::HandleWeaponSelected(FName WeaponId)
{
	if (ALobbyPlayerController* PC = GetOwningPlayer<ALobbyPlayerController>())
	{
		const UMAGameInstance* GI = GetGameInstance<UMAGameInstance>();
		const ULoadoutDataSet* LoadoutDataSet = GI ? GI->TryGetLoadoutDataSet() : nullptr;
		const UDataTable* ResolvedWeaponDataTable = nullptr;
		if (LoadoutDataSet && LoadoutDataSet->WeaponDataTable)
		{
			ResolvedWeaponDataTable = LoadoutDataSet->WeaponDataTable;
		}

		if (!ResolvedWeaponDataTable)
		{
			return;
		}

		const FLoadoutWeaponDataRow* Row = ResolvedWeaponDataTable->FindRow<FLoadoutWeaponDataRow>(WeaponId, TEXT("LoadoutWeaponTab"));
		if (!Row)
		{
			return;
		}

		USkeletalMesh* Mesh = Row->WeaponMesh.LoadSynchronous();
		PC->PreviewWeapon(WeaponId, Mesh, Row->WeaponOffset);
		UpdateSelectedWeapon(WeaponId);
	}
}

void ULoadoutWeaponTabWidget::SyncFromPendingWeapon(FName WeaponId)
{
	UpdateSelectedWeapon(WeaponId);
}

void ULoadoutWeaponTabWidget::UpdateSelectedWeapon(FName WeaponId)
{
	for (ULoadoutWeaponIconButtonWidget* Button : WeaponButtons)
	{
		if (!Button)
		{
			continue;
		}
		Button->SetSelected(Button->WeaponId == WeaponId);
	}
}
