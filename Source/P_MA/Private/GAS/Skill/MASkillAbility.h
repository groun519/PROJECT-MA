#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GAS/Skill/Input/MASkillFlowPart.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"
#include "GameplayTagContainer.h"
#include "MASkillAbility.generated.h"

class UAbilityTask_WaitGameplayEvent;
class UAbilityTask_PlayMontageAndWait;
class UAnimMontage;
class UMASkillDefinition;
class UMASkillEventSource;

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
	void SetRuntimePayload(const FGameplayTag& Key, float Value) { RuntimeContext.SetPayload(Key, Value); }
	void SetRuntimePayload(const FGameplayTag& Key, const FVector& Value) { RuntimeContext.SetPayload(Key, Value); }
	void SetRuntimePayload(const FGameplayTag& Key, UObject* Value) { RuntimeContext.SetPayload(Key, Value); }
	void CompleteCurrentFlow(float MontageBlendOutTime = 0.f);
	bool PrepareNextFlowMontage(float PreviewBlendInTime);
	bool ActivatePreparedNextFlow();

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
	void InitializeFlowParts();
	void UnregisterFlowParts();
	void StartCurrentFlow();
	bool AdvanceToNextFlow(float CurrentFlowMontageBlendOutTime = 0.f);
	void ClearEventTasks();
	void ClearCurrentMontageTask();
	void StopCurrentFlowMontage(float MontageBlendOutTime = 0.f);
	void ClearPreparedMontage();
	void RefreshEventBindings();
	void RegisterCancelTriggers();
	void UnregisterCancelTriggers();
	void HandleCancelTriggerTagChanged(FGameplayTag Tag, int32 NewCount);
	void BindPreparedMontageDelegates(UAnimMontage* Montage);
	void ClearMontageDelegates(UAnimMontage* Montage);
	void HandlePreparedMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	void HandlePreparedMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	int32 ResolveNextMontageFlowIndex(int32 CurrentIndex) const;

	/** RuntimeContext **/
	UPROPERTY(Transient)
	FSkillRuntimeContext RuntimeContext;

	/** Flow Part **/
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMASkillFlowPart>> RuntimeFlowParts;

	UPROPERTY(Transient)
	int32 CurrentFlowIndex = INDEX_NONE;

	UPROPERTY(Transient)
	EMASkillFlowStartMode CurrentFlowStartMode = EMASkillFlowStartMode::Fresh;

	UPROPERTY(Transient)
	int32 PreparedFlowIndex = INDEX_NONE;

	/** Event Sources **/
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMASkillEventSource>> RuntimeEventSources;

	/** Event Part **/
	UPROPERTY(Transient)
	TArray<TObjectPtr<UAbilityTask_WaitGameplayEvent>> EventTasks;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> CurrentMontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> PreparedMontage;

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
