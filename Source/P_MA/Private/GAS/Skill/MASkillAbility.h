// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"
#include "GameplayTagContainer.h"
#include "MASkillAbility.generated.h"

class UAbilityTask_WaitGameplayEvent;
class UAbilityTask_PlayMontageAndWait;
class UMASkillDefinition;
class UMASkillEventSource;
class UMASkillFlowPart;
struct FMASkillGameplayEventPart;

UCLASS()
class P_MA_API UMASkillAbility : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	UMASkillAbility();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	void HandleSkillTagEvent(const FGameplayTag& EventTag);

	const UMASkillDefinition* GetSkillDefinition() const { return SkillDefinition; }
	UMASkillFlowPart* GetCurrentRuntimeFlowPart() const;
	void SetDesiredMontagePlayRate(float NewPlayRate);
	float GetDesiredMontagePlayRate() const { return DesiredMontagePlayRate; }

protected:
	/** Definition DataAsset **/
	UPROPERTY(EditDefaultsOnly, Category="Definition")
	TObjectPtr<UMASkillDefinition> SkillDefinition;

	UPROPERTY(EditDefaultsOnly, Category="Cancel", meta=(Categories="State,Effect"))
	FGameplayTagContainer CancelTriggerTags;

private:
	/** Register **/
	void RegisterEventSources();
	void UnregisterEventSources();
	void RegisterFlowParts();
	void UnregisterFlowParts();
	void StartCurrentFlow();
	bool AdvanceToNextFlow();
	void RefreshEventBindings();
	void RegisterCancelTriggers();
	void UnregisterCancelTriggers();
	void HandleCancelTriggerTagChanged(FGameplayTag Tag, int32 NewCount);
	void HandleCurrentFlowRuntimeEvent(const FGameplayEventData& Payload);

	/** RuntimeContext **/
	UPROPERTY(Transient)
	FSkillRuntimeContext RuntimeContext;

	/** Flow Part **/
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMASkillFlowPart>> RuntimeFlowParts;

	UPROPERTY(Transient)
	int32 CurrentFlowIndex = INDEX_NONE;

	/** Event Sources **/
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMASkillEventSource>> RuntimeEventSources;

	/** Event Part **/
	UPROPERTY(Transient)
	TArray<TObjectPtr<UAbilityTask_WaitGameplayEvent>> EventTasks;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> CurrentMontageTask;

	UPROPERTY(Transient)
	float DesiredMontagePlayRate = 1.f;

	TMap<FGameplayTag, FDelegateHandle> CancelTriggerDelegateHandles;

	UFUNCTION()
	void HandleSkillGameplayEvent(FGameplayEventData Payload);

	UFUNCTION()
	void HandleCurrentFlowMontageCancelled();

	UFUNCTION()
	void HandleCurrentFlowMontageCompleted();

	UFUNCTION()
	void HandleCurrentFlowMontageInterrupted();
};
