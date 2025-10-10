// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "MAP_Dead.generated.h"

/**
 * 
 */
UCLASS()
class UGAP_Dead : public UMAGameplayAbility
{
	GENERATED_BODY()
public:
	UGAP_Dead();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
private:
	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	float RewardRange = 1000.f;

	TArray<AActor*> GetRewardTargets() const;

	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	TSubclassOf<UGameplayEffect> RewardEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	float BaseGoldReward = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	float KillerRewardPortion = 0.5f;

};