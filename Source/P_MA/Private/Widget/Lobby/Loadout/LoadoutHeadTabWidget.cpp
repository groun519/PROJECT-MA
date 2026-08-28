// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutHeadTabWidget.h"
#include "Components/ScrollBox.h"
#include "Engine/DataTable.h"
#include "Framework/MAGameInstance.h"
#include "Player/Loadout/Data/LoadoutDataSet.h"
#include "Widget/Lobby/Loadout/LoadoutColorButtonWidget.h"
#include "Player/Loadout/Data/LoadoutEyeColorPresetData.h"
#include "Player/Loadout/Data/LoadoutEyeShapePresetData.h"
#include "Widget/Lobby/Loadout/LoadoutEyeShapeIconButtonWidget.h"

void ULoadoutHeadTabWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildEyeColorButtons();
	BuildEyeShapeButtons();
}

void ULoadoutHeadTabWidget::BuildEyeColorButtons()
{
	const UMAGameInstance* GI = GetGameInstance<UMAGameInstance>();
	const ULoadoutDataSet* LoadoutDataSet = GI ? GI->TryGetLoadoutDataSet() : nullptr;
	const ULoadoutEyeColorPresetData* ResolvedEyeColorPreset = LoadoutDataSet ? LoadoutDataSet->EyeColorPreset : nullptr;

	if (!EyeColorScrollBox || !EyeColorButtonClass || !ResolvedEyeColorPreset)
	{
		return;
	}

	EyeColorScrollBox->ClearChildren();
	EyeColorButtons.Reset();

	for (const FMaterialParamData& EyeData : ResolvedEyeColorPreset->EyeColors)
	{
		ULoadoutColorButtonWidget* ButtonWidget = CreateWidget<ULoadoutColorButtonWidget>(this, EyeColorButtonClass);
		if (!ButtonWidget)
		{
			continue;
		}

		ButtonWidget->ColorData = EyeData;
		ButtonWidget->OnColorSelected.AddDynamic(this, &ULoadoutHeadTabWidget::HandleEyeColorSelected);
		AddButtonToScrollBox(EyeColorScrollBox, ButtonWidget);

		EyeColorButtons.Add(ButtonWidget);
	}
}

void ULoadoutHeadTabWidget::BuildEyeShapeButtons()
{
	const UMAGameInstance* GI = GetGameInstance<UMAGameInstance>();
	const ULoadoutDataSet* LoadoutDataSet = GI ? GI->TryGetLoadoutDataSet() : nullptr;
	const UDataTable* ResolvedEyeShapeDataTable = nullptr;
	if (LoadoutDataSet && LoadoutDataSet->EyeShapeDataTable)
	{
		ResolvedEyeShapeDataTable = LoadoutDataSet->EyeShapeDataTable;
	}

	if (!EyeShapeScrollBox || !EyeShapeButtonClass || !ResolvedEyeShapeDataTable)
	{
		return;
	}

	EyeShapeScrollBox->ClearChildren();
	EyeShapeButtons.Reset();

	TArray<FName> RowNames = ResolvedEyeShapeDataTable->GetRowNames();
	for (const FName RowName : RowNames)
	{
		const FLoadoutEyeShapeData* EyeShape = ResolvedEyeShapeDataTable->FindRow<FLoadoutEyeShapeData>(RowName, TEXT("LoadoutEyeShapeTab"));
		if (!EyeShape)
		{
			continue;
		}

		ULoadoutEyeShapeIconButtonWidget* ButtonWidget = CreateWidget<ULoadoutEyeShapeIconButtonWidget>(this, EyeShapeButtonClass);
		if (!ButtonWidget)
		{
			continue;
		}

		ButtonWidget->EyeShapeId = RowName;
		ButtonWidget->IconMaterial = EyeShape->IconMaterial;
		ButtonWidget->OnEyeShapeSelected.AddDynamic(this, &ULoadoutHeadTabWidget::HandleEyeShapeSelected);
		AddButtonToScrollBox(EyeShapeScrollBox, ButtonWidget);

		EyeShapeButtons.Add(ButtonWidget);
	}
}

void ULoadoutHeadTabWidget::UpdateSelectedEyeColor(const FMaterialParamData& SelectedData)
{
	for (ULoadoutColorButtonWidget* Button : EyeColorButtons)
	{
		if (!Button)
		{
			continue;
		}

		Button->SetSelected(IsSameColorData(Button->ColorData, SelectedData));
	}
}

void ULoadoutHeadTabWidget::SyncFromPendingHead(const FMaterialParamData& EyeData, FName EyeShapeId)
{
	UpdateSelectedEyeColor(EyeData);
	UpdateSelectedEyeShape(EyeShapeId);
}

void ULoadoutHeadTabWidget::UpdateSelectedEyeShape(FName EyeShapeId)
{
	for (ULoadoutEyeShapeIconButtonWidget* Button : EyeShapeButtons)
	{
		if (!Button)
		{
			continue;
		}

		Button->SetSelected(Button->EyeShapeId == EyeShapeId);
	}
}

void ULoadoutHeadTabWidget::HandleEyeColorSelected(FMaterialParamData SelectedData)
{
	UpdateSelectedEyeColor(SelectedData);
	OnEyeColorSelected.Broadcast(SelectedData);
}

void ULoadoutHeadTabWidget::HandleEyeShapeSelected(FName EyeShapeId)
{
	UpdateSelectedEyeShape(EyeShapeId);
	OnEyeShapeSelected.Broadcast(EyeShapeId);
}
