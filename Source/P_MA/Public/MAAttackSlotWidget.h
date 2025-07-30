// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MAAttackSlotWidget.generated.h"

/**
 * 
 */
class UTextBlock;
class UImage;

UCLASS()
class P_MA_API UMAAttackSlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    void SetHotkeyName(const FString& KeyName);
    void SetSkillIcon(UTexture2D* IconTexture);

protected:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* HotKeyName;

    UPROPERTY(meta = (BindWidget))
    UImage* AttackSlot;
};