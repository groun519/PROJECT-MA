// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"
#include "GameplayTagContainer.h"
#include "MASkillAbility.generated.h"

class UAbilityTask_WaitGameplayEvent;
class UMASkillDefinition;
class UMASkillEventSource;
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
	UMASkillFlowPart* GetRuntimeFlowPart() const { return RuntimeFlowPart; }
	void SetDesiredMontagePlayRate(float NewPlayRate);
	float GetDesiredMontagePlayRate() const { return DesiredMontagePlayRate; }

protected:
	/** Definition DataAsset **/
	UPROPERTY(EditDefaultsOnly, Category="Definition")
	TObjectPtr<UMASkillDefinition> SkillDefinition;

private:
	/** Register **/
	void RegisterEventSources();
	void UnregisterEventSources();
	void RegisterFlowPart();
	void RefreshEventBindings();

	/** RuntimeContext **/
	UPROPERTY(Transient)
	FSkillRuntimeContext RuntimeContext;

	/** Flow Part **/
	UPROPERTY(Transient)
	TObjectPtr<UMASkillFlowPart> RuntimeFlowPart;

	/** Event Sources **/
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMASkillEventSource>> RuntimeEventSources;

	/** Event Part **/
	UPROPERTY(Transient)
	TArray<TObjectPtr<UAbilityTask_WaitGameplayEvent>> EventTasks;

	UPROPERTY(Transient)
	float DesiredMontagePlayRate = 1.f;

	UFUNCTION()
	void HandleSkillGameplayEvent(FGameplayEventData Payload);
};
