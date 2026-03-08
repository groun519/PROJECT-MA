// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GA_BasicAttack_Melee.generated.h"

class UMASkillVFXSet;
/**
 * 
 */
UCLASS()
class UGA_BasicAttack_Melee : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_BasicAttack_Melee();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Animation")
	UAnimMontage* AttackMontage;
	UPROPERTY(EditAnywhere, Category="VFX")
	TObjectPtr<UMASkillVFXSet> VFXDataSet;
	
	UFUNCTION()
	void ComboChangeEventReceived(FGameplayEventData Payload);
	UFUNCTION()
	void DoDamage(FGameplayEventData Payload);
	UFUNCTION()
	void ClearIgnore(FGameplayEventData Payload);
	UFUNCTION()
	void HandleInputPress(float TimeWaited);
	UFUNCTION()
	void HandleVFXEvent(FGameplayEventData Payload);

	void SetupWaitInputPress();
	void TryCommitCombo();

	FName NextComboName;

	UPROPERTY()
	TArray<AActor*> IgnoreActors;
};
