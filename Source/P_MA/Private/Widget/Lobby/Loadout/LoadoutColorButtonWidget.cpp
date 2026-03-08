// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutColorButtonWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"

void ULoadoutColorButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (EquippedBorder)
	{
		EquippedBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
		EquippedBorder->SetOpacity(0.0f);
	}

	if (ColorButton)
	{
		FButtonStyle Style = ColorButton->GetStyle();
		const FLinearColor& Color = ColorData.Color;
		const FLinearColor PressedColor = FLinearColor(
			Color.R * 0.75f,
			Color.G * 0.75f,
			Color.B * 0.75f,
			Color.A
		);
		Style.Normal.TintColor = FSlateColor(Color);
		Style.Hovered.TintColor = FSlateColor(Color);
		Style.Pressed.TintColor = FSlateColor(PressedColor);
		ColorButton->SetStyle(Style);

		ColorButton->OnClicked.AddUniqueDynamic(this, &ULoadoutColorButtonWidget::HandleColorClicked);
	}
}

void ULoadoutColorButtonWidget::HandleColorClicked()
{
	OnColorSelected.Broadcast(ColorData);
}

void ULoadoutColorButtonWidget::SetSelected(bool bInSelected)
{
	if (bSelected == bInSelected)
	{
		return;
	}

	bSelected = bInSelected;

	if (EquippedBorder)
	{
		EquippedBorder->SetOpacity(bSelected ? 1.0f : 0.0f);
	}
}
