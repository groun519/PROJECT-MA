// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Settings/SettingsCategoryButtonWidget.h"

#include "Components/Button.h"

void USettingsCategoryButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CacheButtonStyle();
	Button->OnClicked.AddUniqueDynamic(this, &USettingsCategoryButtonWidget::HandleButtonClicked);
}

void USettingsCategoryButtonWidget::SetSelected(bool bSelected)
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

void USettingsCategoryButtonWidget::CacheButtonStyle()
{
	if (bStyleCached) return;

	BaseButtonStyle = Button->GetStyle();
	bStyleCached = true;
}

void USettingsCategoryButtonWidget::HandleButtonClicked()
{
	OnClicked.Broadcast(Category);
}
