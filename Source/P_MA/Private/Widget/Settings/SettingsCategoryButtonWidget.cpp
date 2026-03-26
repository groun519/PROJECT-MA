// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Settings/SettingsCategoryButtonWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void USettingsCategoryButtonWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	LabelText->SetText(GetCategoryLabelText());
}

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

FText USettingsCategoryButtonWidget::GetCategoryLabelText() const
{
	switch (Category)
	{
	case ESettingsCategory::Graphics:
		return NSLOCTEXT("SettingsCategoryButton", "Graphics", "Graphics");
	case ESettingsCategory::Audio:
		return NSLOCTEXT("SettingsCategoryButton", "Audio", "Audio");
	case ESettingsCategory::Controls:
		return NSLOCTEXT("SettingsCategoryButton", "Controls", "Controls");
	case ESettingsCategory::Gameplay:
		return NSLOCTEXT("SettingsCategoryButton", "Gameplay", "Gameplay");
	default:
		return FText::GetEmpty();
	}
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
