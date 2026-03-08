// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutTabWidgetBase.h"
#include "Components/Widget.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Player/Loadout/LoadoutColorTypes.h"

void ULoadoutTabWidgetBase::AddButtonToScrollBox(UScrollBox* ScrollBox, UWidget* ButtonWidget, const FMargin& Padding)
{
	if (!ScrollBox || !ButtonWidget)
	{
		return;
	}

	if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(ScrollBox->AddChild(ButtonWidget)))
	{
		ScrollSlot->SetPadding(Padding);
	}
}

bool ULoadoutTabWidgetBase::IsSameColorData(const FMaterialParamData& A, const FMaterialParamData& B)
{
	const bool bColorMatch = A.Color.Equals(B.Color, KINDA_SMALL_NUMBER);
	const bool bEmissiveMatch = FMath::IsNearlyEqual(A.Emissive, B.Emissive, KINDA_SMALL_NUMBER);
	return bColorMatch && bEmissiveMatch;
}
