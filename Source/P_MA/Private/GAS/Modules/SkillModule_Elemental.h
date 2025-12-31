// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Modules/MASkillModule.h"
#include "SkillModule_Elemental.generated.h"

/**
 * 
 */
UCLASS()
class USkillModule_Elemental : public UMASkillModule
{
	GENERATED_BODY()

public:
	virtual void CreateAdditionalEffectSpecs(TArray<FGameplayEffectSpecHandle>& OutAdditionalSpecs) const override;

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> StatusEffectClass;

	UPROPERTY(EditAnywhere)
	FLinearColor AttributeColor = FLinearColor::White;
};
