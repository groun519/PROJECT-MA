// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadoutWeaponButtonWidget.generated.h"

class UButton;
class UTexture2D;
class UImage;

UCLASS()
class P_MA_API ULoadoutWeaponButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSelected, FName, WeaponId);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> WeaponButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> EquippedBorder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout|Weapon")
	FName WeaponId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout|Weapon")
	FText WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout|Weapon")
	TSoftObjectPtr<UTexture2D> IconTexture;

	UPROPERTY(BlueprintAssignable, Category = "Loadout|Weapon")
	FOnWeaponSelected OnWeaponSelected;

	void SetSelected(bool bInSelected);
	void SetEquipped(bool bInEquipped);

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleWeaponClicked();

	void ApplyButtonIcon(UTexture2D* Texture);
	void ApplySelectedStyle();

	bool bSelected = false;
	bool bEquipped = false;
	FButtonStyle BaseStyle;
};
