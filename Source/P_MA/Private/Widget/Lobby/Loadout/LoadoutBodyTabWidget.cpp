// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutBodyTabWidget.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
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
	if (!BodyColorScrollBox || !BodyColorButtonClass || !BodyColorPreset)
	{
		return;
	}

	BodyColorScrollBox->ClearChildren();

	for (const FMaterialParamData& BodyData : BodyColorPreset->BodyColors)
	{
		ULoadoutColorButtonWidget* ButtonWidget = CreateWidget<ULoadoutColorButtonWidget>(this, BodyColorButtonClass);
		if (!ButtonWidget)
		{
			continue;
		}

		ButtonWidget->ColorData = BodyData;
		ButtonWidget->OnColorSelected.AddDynamic(this, &ULoadoutBodyTabWidget::HandleBodyColorSelected);
		if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(BodyColorScrollBox->AddChild(ButtonWidget)))
		{
			ScrollSlot->SetPadding(FMargin(6.f, 0.f, 6.f, 0.f));
		}
	}
}

void ULoadoutBodyTabWidget::HandleBodyColorSelected(FMaterialParamData SelectedData)
{
	if (ALobbyPlayerController* PC = GetOwningPlayer<ALobbyPlayerController>())
	{
		PC->PreviewBodyColor(SelectedData);
	}
}
