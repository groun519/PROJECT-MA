// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateTypes.h"
#include "SettingsCategoryButtonWidget.generated.h"

class UButton;

UENUM(BlueprintType)
enum class ESettingsCategory : uint8
{
	Graphics,
	Audio,
	Controls,
	Gameplay
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettingsCategoryButtonClicked, ESettingsCategory, Category);

UCLASS()
class P_MA_API USettingsCategoryButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetSelected(bool bSelected);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	ESettingsCategory Category = ESettingsCategory::Graphics;

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnSettingsCategoryButtonClicked OnClicked;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button;

private:
	FButtonStyle BaseButtonStyle;
	bool bStyleCached = false;

	void CacheButtonStyle();

	UFUNCTION()
	void HandleButtonClicked();
};
