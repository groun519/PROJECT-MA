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

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION(BlueprintPure)
	FVector GetPushDirectionFromEvent(const FGameplayEventData& EventData) const;

	UFUNCTION(BlueprintCallable)
	FName GetFlinchSectionFromEvent(const FGameplayEventData& EventData) const;
};
