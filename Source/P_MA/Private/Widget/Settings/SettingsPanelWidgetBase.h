// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsPanelWidgetBase.generated.h"

class UButton;

UCLASS(Abstract)
class P_MA_API USettingsPanelWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Settings")
	void HandleDefaultRequested();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DefaultButton;
};
