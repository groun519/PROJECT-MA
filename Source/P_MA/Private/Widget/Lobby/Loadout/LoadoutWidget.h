// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/Loadout/LoadoutTypes.h"
#include "Widget/Lobby/Loadout/LoadoutBodyTabWidget.h"
#include "Widget/Lobby/Loadout/LoadoutHeadTabWidget.h"
#include "Widget/Lobby/Loadout/LoadoutMountTabWidget.h"
#include "Widget/Lobby/Loadout/LoadoutWeaponTabWidget.h"
#include "LoadoutWidget.generated.h"

class UButton;
class UWidgetSwitcher;

enum class ELoadoutTab : uint8
{
	Head,
	Body,
	Weapon,
	Mount,
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadoutTabSelected, ELoadoutTab);

UCLASS()
class P_MA_API ULoadoutWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void SyncSelectionFromPending(const FLoadoutSelection& PendingLoadout);
	void ActivateBodyTabUI();

	FOnLoadoutTabSelected& OnTabSelected() { return TabSelectedDelegate; }
	FOnLoadoutBodyColorSelected& OnBodyColorSelected() { return BodyTabWidget->OnBodyColorSelected; }
	FOnLoadoutEyeColorSelected& OnEyeColorSelected() { return HeadTabWidget->OnEyeColorSelected; }
	FOnLoadoutEyeShapeSelected& OnEyeShapeSelected() { return HeadTabWidget->OnEyeShapeSelected; }
	FOnLoadoutWeaponSelected& OnWeaponSelected() { return WeaponTabWidget->OnWeaponSelected; }
	FOnLoadoutMountSelected& OnMountSelected() { return MountTabWidget->OnMountSelected; }

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

	FOnLoadoutTabSelected TabSelectedDelegate;
};
