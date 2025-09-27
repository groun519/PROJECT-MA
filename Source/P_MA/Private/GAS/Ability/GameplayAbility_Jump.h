// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GameplayAbility_Jump.generated.h"

/**
 * 
 */
UCLASS()
class UGameplayAbility_Jump : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_Jump();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* SkillMontage;

	UFUNCTION()
	void OnJumpEventReceived(FGameplayEventData Data);

	enum class EMovementNotifyTags : uint8{None,Start,End};
	FGameplayTag GetJumpTag(EMovementNotifyTags TagType);
};
