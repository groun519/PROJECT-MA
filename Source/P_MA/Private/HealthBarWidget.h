// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h" 

#include "HealthBarWidget.generated.h"

class UProgressBar;

UCLASS()
class P_MA_API UHealthBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetHealthPercent(float HealthPercent);

protected:
    virtual void NativePreConstruct() override;  
    UPROPERTY(meta = (BindWidget))
    UProgressBar* ProgressBar;

    UPROPERTY(EditAnywhere, Category = "Appearance")
    FLinearColor BarColor;
};
