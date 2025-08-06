// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MAPassiveSlotWidget.generated.h"

/**
 * 
 */
class UImage;

UCLASS()
class P_MA_API UMAPassiveSlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    void SetPassiveIcon(UTexture2D* IconTexture);

protected:
    UPROPERTY(meta = (BindWidget))
    UImage* PassiveIcon;
};