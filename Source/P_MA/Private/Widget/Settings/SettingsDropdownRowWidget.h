// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsDropdownRowWidget.generated.h"

class UComboBoxString;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSettingsDropdownSelectionChanged, int32);

UCLASS()
class P_MA_API USettingsDropdownRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetupOptions(const FText& InLabel, const TArray<FText>& InOptions, int32 InSelectedIndex);
	void SetOptionEnabledFlags(const TArray<bool>& InEnabledFlags);
	int32 GetSelectedIndex() const { return SelectedIndex; }

	FOnSettingsDropdownSelectionChanged OnSelectionChanged;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LabelText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> Dropdown;

private:
	TArray<FText> Options;
	TArray<bool> OptionEnabledFlags;
	int32 SelectedIndex = INDEX_NONE;
	bool bUpdatingSelection = false;

	void RefreshOptions();

	UFUNCTION()
	UWidget* HandleGenerateWidget(FString Item);

	UFUNCTION()
	void HandleSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
};
