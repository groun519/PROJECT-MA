// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/Loadout/LoadoutColorTypes.h"
#include "LoadoutColorButtonWidget.generated.h"

class UButton;
class UImage;

UCLASS()
class P_MA_API ULoadoutColorButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnColorSelected, FMaterialParamData, SelectedData);

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ColorButton;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> EquippedBorder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Loadout|Color")
	FMaterialParamData ColorData;

	UPROPERTY(BlueprintAssignable, Category="Loadout|Color")
	FOnColorSelected OnColorSelected;

	void SetEquipped(bool bInEquipped);

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleColorClicked();

	bool bEquipped = false;
};
