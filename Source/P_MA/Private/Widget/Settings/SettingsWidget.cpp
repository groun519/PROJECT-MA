// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Settings/SettingsWidget.h"
#include "Components/WidgetSwitcher.h"
#include "InputCoreTypes.h"
#include "Player/MAPlayerControllerBase.h"

void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);

	GraphicsCategoryButton_0->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleCategoryButtonClicked);
	AudioCategoryButton_1->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleCategoryButtonClicked);
	ControlsCategoryButton_2->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleCategoryButtonClicked);
	GameplayCategoryButton_3->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleCategoryButtonClicked);

	SetActiveCategory(ESettingsCategory::Graphics);
}

FReply USettingsWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		if (AMAPlayerControllerBase* MAPlayerController = GetOwningPlayer<AMAPlayerControllerBase>()) MAPlayerController->CloseSettingsWidget();
		return FReply::Handled();
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void USettingsWidget::SetActiveCategory(ESettingsCategory NewCategory)
{
	SettingsPanelSwitcher->SetActiveWidgetIndex(static_cast<int32>(NewCategory));
	UpdateCategorySelection(NewCategory);
}

void USettingsWidget::UpdateCategorySelection(ESettingsCategory ActiveCategory)
{
	GraphicsCategoryButton_0->SetSelected(ActiveCategory == ESettingsCategory::Graphics);
	AudioCategoryButton_1->SetSelected(ActiveCategory == ESettingsCategory::Audio);
	ControlsCategoryButton_2->SetSelected(ActiveCategory == ESettingsCategory::Controls);
	GameplayCategoryButton_3->SetSelected(ActiveCategory == ESettingsCategory::Gameplay);
}

void USettingsWidget::HandleCategoryButtonClicked(ESettingsCategory Category)
{
	SetActiveCategory(Category);
}
