// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GAP_Jump.generated.h"

/**
 * 
 */
UCLASS()
class UGAP_Jump : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	UGAP_Jump();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	void HandleMovement(const FGameplayEventData* TriggerEventData);
	void HandleDamage(const FGameplayEventData* TriggerEventData);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump Ability")
	float JumpZVel = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump Ability")
	float DamageRadius = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump Ability")
	TSubclassOf<UGameplayEffect> DamageGameplayEffect;
};
