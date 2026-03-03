// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "MAGameplayAbility_ReactionBase.generated.h"

/**
 * 
 */
UCLASS()
class UMAGameplayAbility_ReactionBase : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	UMAGameplayAbility_ReactionBase();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
private:
	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void OnReactDurationEnded();
	
	FVector GetPushDirection(const AActor* Avatar,const AActor* Attacker) const;
	
	UPROPERTY(EditDefaultsOnly, Category="Reaction")
	TMap<FGameplayTag, FGameplayTag> ReactionToDebuffTagMap;
	UPROPERTY(EditDefaultsOnly, Category = "Reaction")
	TMap<FGameplayTag, FGameplayTag> ReactionToImmunityTagMap;

	FGameplayTag CurrentDebuffTag;
	FGameplayTagContainer CancelTagsOnHit;
};
