// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MAOverHeadStatsGauge.generated.h"

/**
 * 
 */
UCLASS()
class UMAOverHeadStatsGauge : public UUserWidget
{
	GENERATED_BODY()
public:
	void ConfigureWithASC(class UAbilitySystemComponent* AbilitySystemComponent);
private:
	UPROPERTY(meta = (BindWidget))
	class UMAValueGauge* HealthBar;

};
