// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutColorButtonWidget.h"
#include "Components/Button.h"

void ULoadoutColorButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ColorButton)
	{
		FButtonStyle Style = ColorButton->GetStyle();
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
	OnColorSelected.Broadcast(Color);
}
