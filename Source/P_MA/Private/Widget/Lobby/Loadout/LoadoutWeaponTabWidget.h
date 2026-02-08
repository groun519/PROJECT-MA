// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadoutWeaponTabWidget.generated.h"

class UDataTable;
class UScrollBox;
class ULoadoutWeaponButtonWidget;

UCLASS()
class P_MA_API ULoadoutWeaponTabWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout|Weapon")
	TObjectPtr<UDataTable> WeaponDataTable;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> WeaponScrollBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout|Weapon")
	TSubclassOf<ULoadoutWeaponButtonWidget> WeaponButtonClass;

	void RefreshEquippedState();

protected:
	virtual void NativeConstruct() override;

private:
	void BuildWeaponButtons();
	void UpdateEquippedWeapon(FName EquippedWeaponId);
	void UpdateSelectedWeapon(FName WeaponId);

	UFUNCTION()
	void HandleWeaponSelected(FName WeaponId);

	UPROPERTY()
	TArray<TObjectPtr<ULoadoutWeaponButtonWidget>> WeaponButtons;
};
