// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GAP_Movement.generated.h"

/**
 * 
 */
UCLASS()
class UGAP_Movement : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	UGAP_Movement();

	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
private:
	UPROPERTY()
	FGameplayTag DashTag = FGameplayTag::RequestGameplayTag("Ability.Passive.Dash.Activate");
	UPROPERTY()
	FGameplayTag RushTag = FGameplayTag::RequestGameplayTag("Ability.Passive.Rush.Activate");
	UPROPERTY()
	FGameplayTag TeleportTag = FGameplayTag::RequestGameplayTag("Ability.Passive.Teleport.Activate");
	UPROPERTY()
	FGameplayTag LaunchTag = FGameplayTag::RequestGameplayTag("Ability.Passive.Launch.Activate");
	UPROPERTY()
	FGameplayTag JumpTag = FGameplayTag::RequestGameplayTag("Ability.Passive.Jump.Activate");
};
