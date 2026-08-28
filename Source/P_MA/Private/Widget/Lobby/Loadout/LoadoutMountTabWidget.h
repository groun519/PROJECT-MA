// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Lobby/Loadout/LoadoutTabWidgetBase.h"
#include "LoadoutMountTabWidget.generated.h"

class UScrollBox;
class ULoadoutMountIconButtonWidget;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadoutMountSelected, FName);

UCLASS()
class P_MA_API ULoadoutMountTabWidget : public ULoadoutTabWidgetBase
{
	GENERATED_BODY()

public:
	FOnLoadoutMountSelected OnMountSelected;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> MountScrollBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout|Mount")
	TSubclassOf<ULoadoutMountIconButtonWidget> MountButtonClass;

	void SyncFromPendingMount(FName MountId);

protected:
	virtual void NativeConstruct() override;

private:
	void BuildMountButtons();
	void UpdateSelectedMount(FName MountId);

	UFUNCTION()
	void HandleMountSelected(FName MountId);

	UPROPERTY()
	TArray<TObjectPtr<ULoadoutMountIconButtonWidget>> MountButtons;
};
