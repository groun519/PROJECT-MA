// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsCheckboxRowWidget.generated.h"

class UCheckBox;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSettingsCheckboxStateChanged, bool);

UCLASS()
class P_MA_API USettingsCheckboxRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetupState(const FText& InLabel, bool bInChecked);
	bool IsChecked() const { return bChecked; }

	FOnSettingsCheckboxStateChanged OnCheckStateChanged;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LabelText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> CheckBox;

private:
	bool bChecked = false;
	bool bUpdatingState = false;

	void RefreshState();

	UFUNCTION()
	void HandleCheckStateChanged(bool bInChecked);
};
