// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAGameplayAbility.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "SkillBehavior_Chain.generated.h"

/**
 * 
 */
UCLASS()
class USkillBehavior_Chain : public UMASkillBehavior
{
	GENERATED_BODY()
	
public:
	virtual void OnActivate_Implementation() override;
	virtual void OnEndAbility_Implementation() override;
	virtual bool IsRequirePlayerInput() const override {return true;}
	
protected:
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitComboChangeEventTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitHitEventTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitClearEventTask;
	TWeakObjectPtr<class UAbilityTask_WaitInputPress> WaitInputPress;

	UFUNCTION()
	void ComboChangedEventReceived(FGameplayEventData EventData);
	UFUNCTION()
	void HitTarget(FGameplayEventData EventData);
	UFUNCTION()
	void ClearIgnore(FGameplayEventData EventData);
	UFUNCTION()
	void HandleInputPress(float Time);

private:
	UPROPERTY(EditDefaultsOnly)
	TMap<FName, TSubclassOf<UGameplayEffect>> DamageEffectMap;
	
	FGameplayTag ComboChangeEventTag = FGameplayTag::RequestGameplayTag("Ability.Combo.Change");
	FGameplayTag ComboEndEventTag = FGameplayTag::RequestGameplayTag("Ability.Combo.Change.End");
	FGameplayTag ComboClearEventTag = FGameplayTag::RequestGameplayTag("Ability.Combo.Clear");
	
	void SetupWaitComboInputPress();
	void TryCommitCombo();
	TSubclassOf<UGameplayEffect> GetDamageEffectForCurrentCombo() const;
	FName NextComboName;
	bool bIsComboInputBuffered;
};
