// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Modules/MASkillModule.h"
#include "SkillModule_Melee.generated.h"

/**
 * 
 */
UCLASS()
class USkillModule_Melee : public UMASkillModule
{
	GENERATED_BODY()

public:
	virtual void OnAbilityActivated() override;
	virtual void OnAbilityEnded(bool bWasCancelled) override;
	virtual void OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData& Payload) override;

protected:
	void PerformMeleeTrace(const FGameplayEventData& Payload);
	
	UPROPERTY()
	FGameplayTag DamageEventTag = FGameplayTag::RequestGameplayTag("Event.Montage.Damage");
};
