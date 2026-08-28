// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutMountTabWidget.h"

#include "Components/ScrollBox.h"
#include "Engine/DataTable.h"
#include "Framework/MAGameInstance.h"
#include "Player/Loadout/Data/LoadoutDataSet.h"
#include "Player/Mount/Data/MountData.h"
#include "Widget/Lobby/Loadout/LoadoutMountIconButtonWidget.h"

void ULoadoutMountTabWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildMountButtons();
}

void ULoadoutMountTabWidget::BuildMountButtons()
{
	const UMAGameInstance* GI = GetGameInstance<UMAGameInstance>();
	const ULoadoutDataSet* LoadoutDataSet = GI ? GI->TryGetLoadoutDataSet() : nullptr;
	const UDataTable* ResolvedMountDataTable = LoadoutDataSet ? LoadoutDataSet->MountDataTable : nullptr;

	if (!MountScrollBox || !MountButtonClass || !ResolvedMountDataTable)
	{
		return;
	}

	MountScrollBox->ClearChildren();
	MountButtons.Reset();

	for (const FName RowName : ResolvedMountDataTable->GetRowNames())
	{
		const FMountDataRow* Row = ResolvedMountDataTable->FindRow<FMountDataRow>(RowName, TEXT("LoadoutMountTab"));
		if (!Row)
		{
			continue;
		}

		ULoadoutMountIconButtonWidget* ButtonWidget = CreateWidget<ULoadoutMountIconButtonWidget>(this, MountButtonClass);
		if (!ButtonWidget)
		{
			continue;
		}

		ButtonWidget->MountId = RowName;
		ButtonWidget->IconMaterial = Row->IconMaterial;
		ButtonWidget->OnMountSelected.AddDynamic(this, &ULoadoutMountTabWidget::HandleMountSelected);
		AddButtonToScrollBox(MountScrollBox, ButtonWidget);

		MountButtons.Add(ButtonWidget);
	}
}

void ULoadoutMountTabWidget::SyncFromPendingMount(FName MountId)
{
	UpdateSelectedMount(MountId);
}

void ULoadoutMountTabWidget::UpdateSelectedMount(FName MountId)
{
	for (ULoadoutMountIconButtonWidget* Button : MountButtons)
	{
		if (!Button)
		{
			continue;
		}

		Button->SetSelected(Button->MountId == MountId);
	}
}

void ULoadoutMountTabWidget::HandleMountSelected(FName MountId)
{
	UpdateSelectedMount(MountId);
	OnMountSelected.Broadcast(MountId);
}
