// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAGameplayAbility.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "SkillBehavior_Charge.generated.h"

/**
 * 
 */
UCLASS()
class USkillBehavior_Charge : public UMASkillBehavior
{
	GENERATED_BODY()

public:
	virtual void OnActivate_Implementation() override;
	virtual void OnEndAbility_Implementation() override;
	virtual bool IsRequirePlayerInput() const override {return true;}
	
protected:
	UFUNCTION()
	void OnChargeEventReceived(FGameplayEventData EventData);
	UFUNCTION()
	void OnMaxCharged();
	UFUNCTION()
	void OnChargeReleased(float Time);
	UFUNCTION()
	void HitTarget(FGameplayEventData EventData);
	
	UPROPERTY(EditDefaultsOnly)
	float MaxChargeDuration = 3.0f;
    
	bool bIsEnd = false;

	TWeakObjectPtr<class UAbilityTask_WaitDelay> ChargeTimeoutTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitSlowTagTask;
	TWeakObjectPtr<class UAbilityTask_WaitInputRelease> InputReleaseTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitHitEventTask;

	FGameplayTag ChargeStartTag = FGameplayTag::RequestGameplayTag("Event.Montage.SlowPlay");
	FGameplayTag DamageEventTag = UMAAbilitySystemStatics::GetMontageDamageTag();
	
	FTimerHandle ChargeUpdateTimerHandle;
	float StartTime = 0.f;
	void UpdateChargeUI();
};
