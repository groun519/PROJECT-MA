// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Lobby/Loadout/LoadoutTabWidgetBase.h"
#include "LoadoutWeaponTabWidget.generated.h"

class UScrollBox;
class UDataTable;
class ULoadoutWeaponIconButtonWidget;
class ULoadoutWeaponModuleButtonWidget;
class UMASkillTooltipWidget;
class UPanelWidget;
struct FLoadoutWeaponDataRow;

UCLASS()
class P_MA_API ULoadoutWeaponTabWidget : public ULoadoutTabWidgetBase
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> WeaponScrollBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout|Weapon")
	TSubclassOf<ULoadoutWeaponIconButtonWidget> WeaponButtonClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> ProvidedModulePanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UMASkillTooltipWidget> ModuleDetailWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout|Weapon")
	TSubclassOf<ULoadoutWeaponModuleButtonWidget> ModuleButtonClass;

	void SyncFromPendingWeapon(FName WeaponId);

protected:
	virtual void NativeConstruct() override;

private:
	void BuildWeaponButtons();
	void UpdateSelectedWeapon(FName WeaponId);
	const FLoadoutWeaponDataRow* FindWeaponData(FName WeaponId) const;
	void RefreshProvidedModules(const FLoadoutWeaponDataRow* WeaponData);
	void SelectProvidedModule(ULoadoutWeaponModuleButtonWidget* ModuleButton);

	UFUNCTION()
	void HandleWeaponSelected(FName WeaponId);
	void HandleProvidedModuleSelected(ULoadoutWeaponModuleButtonWidget* ModuleButton);

	UPROPERTY()
	TArray<TObjectPtr<ULoadoutWeaponIconButtonWidget>> WeaponButtons;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> WeaponDataTable;

	UPROPERTY(Transient)
	TObjectPtr<ULoadoutWeaponModuleButtonWidget> SelectedModuleButton;
};
