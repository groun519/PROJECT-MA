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
	void StartJumpEventReceived(FGameplayEventData Data);

	// 점프의 수직 속도
	UPROPERTY(EditDefaultsOnly, Category = "MA|Jump")
	float JumpZVelocity = 450.0f;

	// 점프의 수평 속도
	UPROPERTY(EditDefaultsOnly, Category = "MA|Jump")
	float JumpXYVelocity = 500.0f;

	enum class EMovementNotifyTags : uint8{None,Start,End};
	FGameplayTag GetJumpTag(EMovementNotifyTags TagType);
};
