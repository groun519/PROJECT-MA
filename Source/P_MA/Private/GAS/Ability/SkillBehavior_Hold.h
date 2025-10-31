// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAGameplayAbility.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "SkillBehavior_Hold.generated.h"

/**
 * 
 */
UCLASS()
class USkillBehavior_Hold : public UMASkillBehavior
{
	GENERATED_BODY()
	
public:
	virtual void OnActivate_Implementation() override;
	virtual void OnEndAbility_Implementation() override;
	virtual bool IsRequirePlayerInput() const override {return true;}

protected:
	UFUNCTION()
	void OnMaxHold();
	UFUNCTION()
	void OnForwardPlay(FGameplayEventData EventData);
	UFUNCTION()
	void OnReversePlay(FGameplayEventData EventData);
	UFUNCTION()
	void OnHoldReleased(float Time);
	UFUNCTION()
	void HitTarget(FGameplayEventData EventData);
	
	UPROPERTY(EditDefaultsOnly)
	float MaxHoldDuration = 3.0f;
	UPROPERTY(EditDefaultsOnly)
	float ReverseSpeed = -2.f;
	
	bool bIsHoldEnd = false;

	TWeakObjectPtr<class UAbilityTask_WaitDelay> HoldTimeOut;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitForwardTagTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitReverseTagTask;
	TWeakObjectPtr<class UAbilityTask_WaitInputRelease> InputReleaseTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitHitEventTask;

	FGameplayTag ReversePlayTag = FGameplayTag::RequestGameplayTag("Event.Montage.ReversePlay");
	FGameplayTag ForwardPlayTag = FGameplayTag::RequestGameplayTag("Event.Montage.ForwardPlay");
	
	FTimerHandle ChargeUpdateTimerHandle;
	float StartTime = 0.f;
	void UpdateChargeUI();
};
