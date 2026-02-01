// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/Loadout/LoadoutColorTypes.h"
#include "LoadoutBodyTabWidget.generated.h"

class ULoadoutBodyColorPresetData;
class UScrollBox;
class ULoadoutColorButtonWidget;

UCLASS()
class P_MA_API ULoadoutBodyTabWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout|Body")
	TObjectPtr<ULoadoutBodyColorPresetData> BodyColorPreset;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UScrollBox> BodyColorScrollBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout|Body")
	TSubclassOf<ULoadoutColorButtonWidget> BodyColorButtonClass;

protected:
	virtual void NativeConstruct() override;

private:
	void BuildBodyColorButtons();

	UFUNCTION()
	void HandleBodyColorSelected(FMaterialParamData SelectedData);
};
