// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Lobby/Loadout/LoadoutIconButtonWidget.h"
#include "LoadoutEyeShapeIconButtonWidget.generated.h"

UCLASS()
class P_MA_API ULoadoutEyeShapeIconButtonWidget : public ULoadoutIconButtonWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEyeShapeSelected, FName, EyeShapeId);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout|Head")
	FName EyeShapeId = NAME_None;

	UPROPERTY(BlueprintAssignable, Category = "Loadout|Head")
	FOnEyeShapeSelected OnEyeShapeSelected;

protected:
	virtual void OnButtonClicked() override;
};
