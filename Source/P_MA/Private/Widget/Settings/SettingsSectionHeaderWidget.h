// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsSectionHeaderWidget.generated.h"

class UTextBlock;

UCLASS()
class P_MA_API USettingsSectionHeaderWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupHeader(const FText& InText);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HeaderText;
};
