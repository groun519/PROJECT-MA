// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "GAS/Modules/MASkillModule.h"
#include "SkillModule_Combo.generated.h"

/**
 * 
 */
UCLASS()
class USkillModule_Combo : public UMASkillModule
{
	GENERATED_BODY()

public:
	virtual void OnAbilityActivated() override;
	virtual void OnAbilityEnded(bool bWasCancelled) override;
	virtual void ModifyDamageSpec(FGameplayEffectSpecHandle& SpecHandle) const override;
	virtual void CreateAdditionalEffectSpecs(TArray<FGameplayEffectSpecHandle>& OutAdditionalSpecs) const override;

	bool TryActivateCombo();

protected:
	UFUNCTION()
	void OnComboInputPressed(float TimeWaited);

	void StartComboMontage();

	UFUNCTION()
	void OnComboMontageEnded();

	UFUNCTION()
	void OnComboDamageEvent(FGameplayEventData Payload);

	UFUNCTION()
	void OnInputWindowEnded();

private:
	bool bComboInputPressed = false;
	bool bIsComboActive = false;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitInputPress> InputPressTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitDelay> InputWindowTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> ComboMontageTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> DamageEventTask;
};
