// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"
#include "GameplayTagContainer.h"
#include "MASkillAbility.generated.h"

class UAbilityTask_WaitGameplayEvent;
class UMASkillDefinition;
class UMASkillFlowPart;
struct FMASkillGameplayEventPart;

UCLASS()
class P_MA_API UMASkillAbility : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	const UMASkillDefinition* GetSkillDefinition() const { return SkillDefinition; }

protected:
	/** Definition **/
	UPROPERTY(EditDefaultsOnly, Category="Definition")
	TObjectPtr<UMASkillDefinition> SkillDefinition;

private:
	/** Register **/
	void RegisterFlowPart();
	void RegisterEventParts();

	UPROPERTY(Transient)
	FSkillRuntimeContext RuntimeContext;

	/** Flow Part **/
	UPROPERTY(Transient)
	TObjectPtr<UMASkillFlowPart> RuntimeFlowPart;

	/** Event Part **/
	UPROPERTY(Transient)
	TArray<TObjectPtr<UAbilityTask_WaitGameplayEvent>> EventTasks;

	UFUNCTION()
	void HandleSkillGameplayEvent(FGameplayEventData Payload);
};
