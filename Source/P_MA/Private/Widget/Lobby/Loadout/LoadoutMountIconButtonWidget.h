// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Lobby/Loadout/LoadoutIconButtonWidget.h"
#include "LoadoutMountIconButtonWidget.generated.h"

UCLASS()
class P_MA_API ULoadoutMountIconButtonWidget : public ULoadoutIconButtonWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMountSelected, FName, MountId);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout|Mount")
	FName MountId;

	UPROPERTY(BlueprintAssignable, Category = "Loadout|Mount")
	FOnMountSelected OnMountSelected;

protected:
	virtual void OnButtonClicked() override;
};
