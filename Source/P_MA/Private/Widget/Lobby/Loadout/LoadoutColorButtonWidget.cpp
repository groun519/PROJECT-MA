// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutColorButtonWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"

void ULoadoutColorButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

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

		ColorButton->OnClicked.AddDynamic(this, &ULoadoutColorButtonWidget::HandleColorClicked);
	}
}

void ULoadoutColorButtonWidget::HandleColorClicked()
{
	OnColorSelected.Broadcast(ColorData);
}

void ULoadoutColorButtonWidget::SetEquipped(bool bInEquipped)
{
	if (bEquipped == bInEquipped)
	{
		return;
	}

	bEquipped = bInEquipped;
	if (ColorButton)
	{
		ColorButton->SetIsEnabled(!bEquipped);
	}

	if (EquippedBorder)
	{
		EquippedBorder->SetVisibility(bEquipped ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}
