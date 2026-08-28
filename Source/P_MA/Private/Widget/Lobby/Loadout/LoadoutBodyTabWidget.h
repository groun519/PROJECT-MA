// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Lobby/Loadout/LoadoutTabWidgetBase.h"
#include "Player/Loadout/LoadoutTypes.h"
#include "LoadoutBodyTabWidget.generated.h"

class UScrollBox;
class ULoadoutColorButtonWidget;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadoutBodyColorSelected, const FMaterialParamData&);

UCLASS()
class P_MA_API ULoadoutBodyTabWidget : public ULoadoutTabWidgetBase
{
	GENERATED_BODY()

public:
	FOnLoadoutBodyColorSelected OnBodyColorSelected;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UScrollBox> BodyColorScrollBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout|Body")
	TSubclassOf<ULoadoutColorButtonWidget> BodyColorButtonClass;

	void SyncFromPendingBody(const FMaterialParamData& BodyData);

protected:
	virtual void NativeConstruct() override;

private:
	void BuildBodyColorButtons();
	void UpdateSelectedBodyColor(const FMaterialParamData& SelectedData);

	UFUNCTION()
	void HandleBodyColorSelected(FMaterialParamData SelectedData);

	UPROPERTY()
	TArray<TObjectPtr<ULoadoutColorButtonWidget>> BodyColorButtons;
};
