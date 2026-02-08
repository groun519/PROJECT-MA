// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutHeadTabWidget.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Widget/Lobby/Loadout/LoadoutColorButtonWidget.h"
#include "Player/Loadout/Data/LoadoutEyeColorPresetData.h"
#include "Level/Lobby/LobbyPlayerController.h"
#include "Player/MAPlayerState.h"

void ULoadoutHeadTabWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildEyeColorButtons();
	RefreshEquippedState();
}

void ULoadoutHeadTabWidget::BuildEyeColorButtons()
{
	if (!EyeColorScrollBox || !EyeColorButtonClass || !EyeColorPreset)
	{
		return;
	}

	EyeColorScrollBox->ClearChildren();
	EyeColorButtons.Reset();

	for (const FMaterialParamData& EyeData : EyeColorPreset->EyeColors)
	{
		ULoadoutColorButtonWidget* ButtonWidget = CreateWidget<ULoadoutColorButtonWidget>(this, EyeColorButtonClass);
		if (!ButtonWidget)
		{
			continue;
		}

		ButtonWidget->ColorData = EyeData;
		ButtonWidget->OnColorSelected.AddDynamic(this, &ULoadoutHeadTabWidget::HandleEyeColorSelected);
		if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(EyeColorScrollBox->AddChild(ButtonWidget)))
		{
			ScrollSlot->SetPadding(FMargin(6.f, 0.f, 6.f, 0.f));
		}

		EyeColorButtons.Add(ButtonWidget);
	}
}

void ULoadoutHeadTabWidget::RefreshEquippedState()
{
	if (ALobbyPlayerController* PC = GetOwningPlayer<ALobbyPlayerController>())
	{
		if (AMAPlayerState* PS = PC->GetPlayerState<AMAPlayerState>())
		{
			UpdateEquippedEyeColor(PS->GetLoadoutColor().EyeData);
		}
	}
}

void ULoadoutHeadTabWidget::UpdateEquippedEyeColor(const FMaterialParamData& EquippedData)
{
	for (ULoadoutColorButtonWidget* Button : EyeColorButtons)
	{
		if (!Button)
		{
			continue;
		}

		Button->SetEquipped(IsSameColor(Button->ColorData, EquippedData));
	}
}

bool ULoadoutHeadTabWidget::IsSameColor(const FMaterialParamData& A, const FMaterialParamData& B)
{
	const bool bColorMatch = A.Color.Equals(B.Color, KINDA_SMALL_NUMBER);
	const bool bEmissiveMatch = FMath::IsNearlyEqual(A.Emissive, B.Emissive, KINDA_SMALL_NUMBER);
	return bColorMatch && bEmissiveMatch;
}

void ULoadoutHeadTabWidget::HandleEyeColorSelected(FMaterialParamData SelectedData)
{
	if (ALobbyPlayerController* PC = GetOwningPlayer<ALobbyPlayerController>())
	{
		PC->PreviewEyeColor(SelectedData);
	}
}
