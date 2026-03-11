// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Lobby/Loadout/LoadoutIconButtonWidget.h"
#include "LoadoutWeaponIconButtonWidget.generated.h"

UCLASS()
class P_MA_API ULoadoutWeaponIconButtonWidget : public ULoadoutIconButtonWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSelected, FName, WeaponId);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout|Weapon")
	FName WeaponId;

	UPROPERTY(BlueprintAssignable, Category = "Loadout|Weapon")
	FOnWeaponSelected OnWeaponSelected;

protected:
	virtual void OnButtonClicked() override;
};
