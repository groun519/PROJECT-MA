// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/Loadout/LoadoutColorTypes.h"
#include "LoadoutHeadTabWidget.generated.h"

class ULoadoutEyeColorPresetData;
class UScrollBox;
class ULoadoutColorButtonWidget;

UCLASS()
class P_MA_API ULoadoutHeadTabWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout|Head")
	TObjectPtr<ULoadoutEyeColorPresetData> EyeColorPreset;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UScrollBox> EyeColorScrollBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout|Head")
	TSubclassOf<ULoadoutColorButtonWidget> EyeColorButtonClass;

protected:
	virtual void NativeConstruct() override;

private:
	void BuildEyeColorButtons();
	void UpdateSelectedEyeColor(const FMaterialParamData& SelectedData);
	static bool IsSameColor(const FMaterialParamData& A, const FMaterialParamData& B);

	UFUNCTION()
	void HandleEyeColorSelected(FMaterialParamData SelectedData);

	UPROPERTY()
	TArray<TObjectPtr<ULoadoutColorButtonWidget>> EyeColorButtons;
};
