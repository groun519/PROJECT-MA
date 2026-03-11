// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Lobby/Loadout/LoadoutTabWidgetBase.h"
#include "LoadoutWeaponTabWidget.generated.h"

class UScrollBox;
class ULoadoutWeaponIconButtonWidget;

UCLASS()
class P_MA_API ULoadoutWeaponTabWidget : public ULoadoutTabWidgetBase
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> WeaponScrollBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout|Weapon")
	TSubclassOf<ULoadoutWeaponIconButtonWidget> WeaponButtonClass;

	void SyncFromPendingWeapon(FName WeaponId);

protected:
	virtual void NativeConstruct() override;

private:
	void BuildWeaponButtons();
	void UpdateSelectedWeapon(FName WeaponId);

	UFUNCTION()
	void HandleWeaponSelected(FName WeaponId);

	UPROPERTY()
	TArray<TObjectPtr<ULoadoutWeaponIconButtonWidget>> WeaponButtons;
};
