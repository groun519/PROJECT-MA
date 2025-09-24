// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GameplayAbility_VolcanoEruption.generated.h"

/**
 * 
 */
UCLASS()
class UGameplayAbility_VolcanoEruption : public UMAGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGameplayAbility_VolcanoEruption();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
private:
    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    TSubclassOf<UGameplayEffect> SkillDamageEffect;
    
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* SkillMontage;

	UFUNCTION()
	void DoDamage(FGameplayEventData EventData);

	UFUNCTION()
	void JumpToTarget(FGameplayEventData EventData);

	static FGameplayTag GetVolcanoEruptionDamageTag();

	FVector TargetLocation;
};
