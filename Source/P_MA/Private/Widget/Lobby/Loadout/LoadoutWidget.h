// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/Loadout/LoadoutTypes.h"
#include "LoadoutWidget.generated.h"

class UButton;
class UWidgetSwitcher;
class ULoadoutHeadTabWidget;
class ULoadoutBodyTabWidget;
class ULoadoutWeaponTabWidget;
class ULoadoutMountTabWidget;

UCLASS()
class P_MA_API ULoadoutWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void SyncSelectionFromPending(const FLoadoutSelection& PendingLoadout);
	void ActivateBodyTabUI();

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> HeadTabButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> BodyTabButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> WeaponTabButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> MountTabButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWidgetSwitcher> TabSwitcher;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<ULoadoutHeadTabWidget> HeadTabWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<ULoadoutBodyTabWidget> BodyTabWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<ULoadoutWeaponTabWidget> WeaponTabWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<ULoadoutMountTabWidget> MountTabWidget;

private:
	UFUNCTION()
	void HandleHeadTabClicked();

	UFUNCTION()
	void HandleBodyTabClicked();

	UFUNCTION()
	void HandleWeaponTabClicked();

	UFUNCTION()
	void HandleMountTabClicked();

	void SetActiveTab(int32 TabIndex);
};
