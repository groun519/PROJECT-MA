// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Settings/SettingsToggleButtonWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void USettingsToggleButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CacheButtonStyle();
	Button->OnClicked.AddUniqueDynamic(this, &USettingsToggleButtonWidget::HandleButtonClicked);
}

void USettingsToggleButtonWidget::SetLabel(const FText& InText)
{
	LabelText->SetText(InText);
}

void USettingsToggleButtonWidget::SetSelected(bool bSelected)
{
	CacheButtonStyle();

	FButtonStyle ButtonStyle = BaseButtonStyle;
	if (bSelected)
	{
		ButtonStyle.SetNormal(BaseButtonStyle.Pressed);
		ButtonStyle.SetHovered(BaseButtonStyle.Pressed);
	}

	Button->SetStyle(ButtonStyle);
}

void USettingsToggleButtonWidget::CacheButtonStyle()
{
	if (bStyleCached) return;

	BaseButtonStyle = Button->GetStyle();
	bStyleCached = true;
}

void USettingsToggleButtonWidget::HandleButtonClicked()
{
	OnClicked.Broadcast(ButtonIndex);
}
