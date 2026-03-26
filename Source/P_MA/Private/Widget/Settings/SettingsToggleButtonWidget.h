// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateTypes.h"
#include "SettingsToggleButtonWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSettingsToggleButtonClicked, int32);

UCLASS()
class P_MA_API USettingsToggleButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetLabel(const FText& InText);
	void SetSelected(bool bSelected);
	void SetButtonIndex(int32 InIndex) { ButtonIndex = InIndex; }
	int32 GetButtonIndex() const { return ButtonIndex; }

	FOnSettingsToggleButtonClicked OnClicked;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LabelText;

private:
	FButtonStyle BaseButtonStyle;
	bool bStyleCached = false;
	int32 ButtonIndex = INDEX_NONE;

	void CacheButtonStyle();

	UFUNCTION()
	void HandleButtonClicked();
};
