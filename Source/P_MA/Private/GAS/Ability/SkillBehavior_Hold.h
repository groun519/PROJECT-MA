// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	virtual bool IsApplyCooldownImmediate() const override {return false;}
	virtual void InitFromConfig(const FInstancedStruct& ConfigPayload) override;

protected:
	bool bCanMoveWhileHolding = false;
	
	float MaxHoldDuration;
private:
	
	bool bIsHoldEnd = false;
	FTimerHandle ChargeUpdateTimerHandle;
	float StartTime = 0.f;
	void UpdateChargeUI();

	TWeakObjectPtr<class UAbilityTask_WaitDelay> HoldTimeOut;
	TWeakObjectPtr<class UAbilityTask_WaitInputRelease> InputReleaseTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitHitEventTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitClearEventTask;
	
	UFUNCTION()
	void OnMaxHold();
	UFUNCTION()
	void OnHoldReleased(float Time);
	UFUNCTION()
	void HitTarget(FGameplayEventData EventData);
	UFUNCTION()
	void ClearIgnore(FGameplayEventData EventData);
	UFUNCTION()
	void JumpSection(FGameplayEventData Payload);

	FGameplayTag MontageJumpTag = FGameplayTag::RequestGameplayTag("Event.Montage.JumpSection");
};
