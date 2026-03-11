// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutBodyTabWidget.h"
#include "Components/ScrollBox.h"
#include "Framework/MAGameInstance.h"
#include "Player/Loadout/Data/LoadoutDataSet.h"
#include "Widget/Lobby/Loadout/LoadoutColorButtonWidget.h"
#include "Player/Loadout/Data/LoadoutBodyColorPresetData.h"
#include "Level/Lobby/LobbyPlayerController.h"

void ULoadoutBodyTabWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildBodyColorButtons();
}

void ULoadoutBodyTabWidget::BuildBodyColorButtons()
{
	const UMAGameInstance* GI = GetGameInstance<UMAGameInstance>();
	const ULoadoutDataSet* LoadoutDataSet = GI ? GI->TryGetLoadoutDataSet() : nullptr;
	const ULoadoutBodyColorPresetData* ResolvedBodyColorPreset = LoadoutDataSet ? LoadoutDataSet->BodyColorPreset : nullptr;

	if (!BodyColorScrollBox || !BodyColorButtonClass || !ResolvedBodyColorPreset)
	{
		return;
	}

	BodyColorScrollBox->ClearChildren();
	BodyColorButtons.Reset();

	for (const FMaterialParamData& BodyData : ResolvedBodyColorPreset->BodyColors)
	{
		ULoadoutColorButtonWidget* ButtonWidget = CreateWidget<ULoadoutColorButtonWidget>(this, BodyColorButtonClass);
		if (!ButtonWidget)
		{
			continue;
		}

		ButtonWidget->ColorData = BodyData;
		ButtonWidget->OnColorSelected.AddDynamic(this, &ULoadoutBodyTabWidget::HandleBodyColorSelected);
		AddButtonToScrollBox(BodyColorScrollBox, ButtonWidget);

		BodyColorButtons.Add(ButtonWidget);
	}
}

void ULoadoutBodyTabWidget::SyncFromPendingBody(const FMaterialParamData& BodyData)
{
	UpdateSelectedBodyColor(BodyData);
}

void ULoadoutBodyTabWidget::UpdateSelectedBodyColor(const FMaterialParamData& SelectedData)
{
	for (ULoadoutColorButtonWidget* Button : BodyColorButtons)
	{
		if (!Button)
		{
			continue;
		}

		Button->SetSelected(IsSameColorData(Button->ColorData, SelectedData));
	}
}

void ULoadoutBodyTabWidget::HandleBodyColorSelected(FMaterialParamData SelectedData)
{
	if (ALobbyPlayerController* PC = GetOwningPlayer<ALobbyPlayerController>())
	{
		PC->PreviewBodyColor(SelectedData);
	}

	UpdateSelectedBodyColor(SelectedData);
}
