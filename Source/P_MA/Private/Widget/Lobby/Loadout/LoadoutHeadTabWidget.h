// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Lobby/Loadout/LoadoutTabWidgetBase.h"
#include "Player/Loadout/LoadoutColorTypes.h"
#include "LoadoutHeadTabWidget.generated.h"

class UScrollBox;
class ULoadoutColorButtonWidget;
class ULoadoutEyeShapeIconButtonWidget;

UCLASS()
class P_MA_API ULoadoutHeadTabWidget : public ULoadoutTabWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	
public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UScrollBox> EyeColorScrollBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout|Head")
	TSubclassOf<ULoadoutColorButtonWidget> EyeColorButtonClass;

	/** Eye Shape **/
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UScrollBox> EyeShapeScrollBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout|Head")
	TSubclassOf<ULoadoutEyeShapeIconButtonWidget> EyeShapeButtonClass;

	void SyncFromPendingHead(const FMaterialParamData& EyeData, FName EyeShapeId);

private:
	/** Eye Color **/
	void BuildEyeColorButtons();
	void UpdateSelectedEyeColor(const FMaterialParamData& SelectedData);

	UFUNCTION()
	void HandleEyeColorSelected(FMaterialParamData SelectedData);

	UPROPERTY()
	TArray<TObjectPtr<ULoadoutColorButtonWidget>> EyeColorButtons;

	/** Eye Shape **/
	void BuildEyeShapeButtons();
	void UpdateSelectedEyeShape(FName EyeShapeId);

	UFUNCTION()
	void HandleEyeShapeSelected(FName EyeShapeId);

	UPROPERTY()
	TArray<TObjectPtr<ULoadoutEyeShapeIconButtonWidget>> EyeShapeButtons;
};
