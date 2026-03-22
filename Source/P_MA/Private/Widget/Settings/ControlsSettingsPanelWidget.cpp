// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Settings/ControlsSettingsPanelWidget.h"

#include "Components/VerticalBox.h"
#include "EnhancedActionKeyMapping.h"
#include "InputMappingContext.h"
#include "Widget/Settings/SettingsKeyBindingRowWidget.h"
#include "Widget/Settings/SettingsSectionHeaderWidget.h"

namespace
{
	struct FBindingRowData
	{
		FText DisplayName;
		FText DisplayCategory;
		FText PrimaryKeyText;
		FText SecondaryKeyText;
		bool bHasSecondaryKey = false;
	};

	FText GetBindingDisplayName(const FEnhancedActionKeyMapping& Mapping)
	{
		return Mapping.GetDisplayName().IsEmpty() ? FText::FromName(Mapping.GetMappingName()) : Mapping.GetDisplayName();
	}

	FText GetBindingKeyText(const FKey& Key)
	{
		return Key.IsValid() ? Key.GetDisplayName(false) : FText::GetEmpty();
	}
}

void UControlsSettingsPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RebuildBindingRows();
}

void UControlsSettingsPanelWidget::RebuildBindingRows()
{
	const UControlsSettingsPanelWidget* DefaultWidget = GetClass()->GetDefaultObject<UControlsSettingsPanelWidget>();
	const TSubclassOf<USettingsKeyBindingRowWidget> EffectiveKeyBindingRowClass =
		KeyBindingRowClass ? KeyBindingRowClass : (DefaultWidget ? DefaultWidget->KeyBindingRowClass : nullptr);
	const TSubclassOf<USettingsSectionHeaderWidget> EffectiveCategoryHeaderWidgetClass =
		CategoryHeaderWidgetClass ? CategoryHeaderWidgetClass : (DefaultWidget ? DefaultWidget->CategoryHeaderWidgetClass : nullptr);
	const TArray<TObjectPtr<UInputMappingContext>>* EffectiveSourceContexts =
		SourceContexts.Num() > 0 ? &SourceContexts : (DefaultWidget ? &DefaultWidget->SourceContexts : nullptr);

	if (!BindingRowsBox || !EffectiveKeyBindingRowClass || !GetOwningPlayer()) return;

	BindingRowsBox->ClearChildren();
	FString LastCategoryName;
	TArray<FBindingRowData> BindingRows;
	TMap<FName, int32> BindingRowIndexByName;

	for (const UInputMappingContext* Context : *EffectiveSourceContexts)
	{
		if (!Context) continue;

		for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings())
		{
			if (!Mapping.IsPlayerMappable()) continue;

			const FName MappingName = Mapping.GetMappingName();
			if (MappingName.IsNone()) continue;
			const FText KeyText = GetBindingKeyText(Mapping.Key);
			if (int32* ExistingRowIndex = BindingRowIndexByName.Find(MappingName))
			{
				FBindingRowData& ExistingRow = BindingRows[*ExistingRowIndex];
				if (!ExistingRow.bHasSecondaryKey)
				{
					ExistingRow.SecondaryKeyText = KeyText;
					ExistingRow.bHasSecondaryKey = true;
				}
				continue;
			}

			FBindingRowData& NewRow = BindingRows.AddDefaulted_GetRef();
			NewRow.DisplayName = GetBindingDisplayName(Mapping);
			NewRow.DisplayCategory = Mapping.GetDisplayCategory();
			NewRow.PrimaryKeyText = KeyText;
			BindingRowIndexByName.Add(MappingName, BindingRows.Num() - 1);
		}
	}

	for (const FBindingRowData& BindingRow : BindingRows)
	{
		const FText DisplayCategory = BindingRow.DisplayCategory;
		const FString DisplayCategoryString = DisplayCategory.ToString();
		if (EffectiveCategoryHeaderWidgetClass && !DisplayCategoryString.IsEmpty() && DisplayCategoryString != LastCategoryName)
		{
			USettingsSectionHeaderWidget* HeaderRow = CreateWidget<USettingsSectionHeaderWidget>(GetOwningPlayer(), EffectiveCategoryHeaderWidgetClass);
			if (HeaderRow)
			{
				HeaderRow->SetupHeader(DisplayCategory);
				BindingRowsBox->AddChild(HeaderRow);
			}
			LastCategoryName = DisplayCategoryString;
		}

		USettingsKeyBindingRowWidget* Row = CreateWidget<USettingsKeyBindingRowWidget>(GetOwningPlayer(), EffectiveKeyBindingRowClass);
		if (!Row) continue;

		Row->SetupRow(BindingRow.DisplayName, BindingRow.PrimaryKeyText);
		if (BindingRow.bHasSecondaryKey) Row->SetupSecondaryKey(BindingRow.SecondaryKeyText);
		BindingRowsBox->AddChild(Row);
	}
}
