// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutBodyTabWidget.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Widget/Lobby/Loadout/LoadoutColorButtonWidget.h"
#include "Player/Loadout/Data/LoadoutBodyColorPresetData.h"
#include "Level/Lobby/LobbyPlayerController.h"
#include "Player/MAPlayerState.h"

void ULoadoutBodyTabWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildBodyColorButtons();
	RefreshEquippedState();
}

void ULoadoutBodyTabWidget::BuildBodyColorButtons()
{
	if (!BodyColorScrollBox || !BodyColorButtonClass || !BodyColorPreset)
	{
		return;
	}

	BodyColorScrollBox->ClearChildren();
	BodyColorButtons.Reset();

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

		BodyColorButtons.Add(ButtonWidget);
	}
}

void ULoadoutBodyTabWidget::RefreshEquippedState()
{
	if (ALobbyPlayerController* PC = GetOwningPlayer<ALobbyPlayerController>())
	{
		if (AMAPlayerState* PS = PC->GetPlayerState<AMAPlayerState>())
		{
			UpdateEquippedBodyColor(PS->GetLoadoutColor().BodyData);
		}
	}
}

void ULoadoutBodyTabWidget::UpdateEquippedBodyColor(const FMaterialParamData& EquippedData)
{
	for (ULoadoutColorButtonWidget* Button : BodyColorButtons)
	{
		if (!Button)
		{
			continue;
		}

		Button->SetEquipped(IsSameColor(Button->ColorData, EquippedData));
	}
}

bool ULoadoutBodyTabWidget::IsSameColor(const FMaterialParamData& A, const FMaterialParamData& B)
{
	const bool bColorMatch = A.Color.Equals(B.Color, KINDA_SMALL_NUMBER);
	const bool bEmissiveMatch = FMath::IsNearlyEqual(A.Emissive, B.Emissive, KINDA_SMALL_NUMBER);
	return bColorMatch && bEmissiveMatch;
}

void ULoadoutBodyTabWidget::HandleBodyColorSelected(FMaterialParamData SelectedData)
{
	if (ALobbyPlayerController* PC = GetOwningPlayer<ALobbyPlayerController>())
	{
		PC->PreviewBodyColor(SelectedData);
	}
}
