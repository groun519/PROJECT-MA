#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GAS/Skill/Step/MASkillStep.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"
#include "GameplayTagContainer.h"
#include "MASkillAbility.generated.h"

class UAbilityTask_WaitGameplayEvent;
class UAbilityTask_PlayMontageAndWait;
class UAnimMontage;
class UAnimSequenceBase;
class UDataTable;
class UMASkillDefinition;
class UMASkillEventSource;
class UMASkillAction;

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

	const UMASkillDefinition* GetSkillDefinition() const;
	const FGameplayTag& GetElementalTag() const;
	const UDataTable* GetElementalDataTable() const { return ElementalDataTable; }
	const UDataTable* GetOverlapDecalDataTable() const { return OverlapDecalDataTable; }
	UMASkillStep* GetCurrentRuntimeSkillStep() const;
	FMASkillPayloadStore& GetPayloadStore() { return PayloadStore; }
	const FMASkillPayloadStore& GetPayloadStore() const { return PayloadStore; }
	void SetDesiredMontagePlayRate(float NewPlayRate);
	float GetDesiredMontagePlayRate() const { return DesiredMontagePlayRate; }
	void MultiplyFinalDamageMultiplier(float Multiplier) { RuntimeContext.MultiplyFinalDamageMultiplier(Multiplier); }
	void CompleteCurrentStep(float MontageBlendOutTime = 0.f);
	bool PrepareNextStepMontage(float PreviewBlendInTime);
	bool ActivatePreparedNextStep();

protected:
	/** Definition DataAsset **/
	UPROPERTY(EditDefaultsOnly, Category="Definition")
	TObjectPtr<UMASkillDefinition> SkillDefinition;

	// TODO: Move this kind of shared lookup data into a common subsystem once the
	// skill runtime starts depending on more global registries than elemental data.
	UPROPERTY(EditDefaultsOnly, Category="Elemental", meta=(RowType="/Script/P_MA.MAElementDataRow"))
	TObjectPtr<UDataTable> ElementalDataTable;

	UPROPERTY(EditDefaultsOnly, Category="Effect", meta=(RowType="/Script/P_MA.MAOverlapDecalDataRow"))
	TObjectPtr<UDataTable> OverlapDecalDataTable;

	UPROPERTY(EditDefaultsOnly, Category="Cancel", meta=(Categories="State,Effect"))
	FGameplayTagContainer CancelTriggerTags;

private:
	/** Register **/
	void RegisterEventSources();
	void UnregisterEventSources();
	void RegisterSkillSteps();
	void UnregisterSkillSteps();
	void ResetStepExecutionState(float CurrentStepMontageBlendOutTime = 0.f);
	void StartCurrentStep();
	bool AdvanceToNextStep(float CurrentStepMontageBlendOutTime = 0.f);
	void ClearEventTasks();
	void ClearCurrentMontageTask();
	void StopCurrentStepMontage(float MontageBlendOutTime = 0.f);
	void ClearPreparedMontage();
	void RefreshEventBindings();
	void RegisterCancelTriggers();
	void UnregisterCancelTriggers();
	void HandleCancelTriggerTagChanged(FGameplayTag Tag, int32 NewCount);
	void BindPreparedMontageDelegates(UAnimMontage* Montage);
	void ClearMontageDelegates(UAnimMontage* Montage);
	void RegisterAnimationOwner(UAnimSequenceBase* Animation);
	void UnregisterAnimationOwner(UAnimSequenceBase* Animation);
	void ResetResolvedData();
	void ResolveActionsForEvent(const FGameplayTag& EventTag, TArray<UMASkillAction*>& OutActions) const;
	void HandlePreparedMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	void HandlePreparedMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void CacheRuntimeSkillDefinition(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo);

	/** RuntimeContext **/
	UPROPERTY(Transient)
	FSkillRuntimeContext RuntimeContext;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillDefinition> RuntimeSkillDefinition;

	UPROPERTY(Transient)
	FMASkillPayloadStore PayloadStore;

	TSet<FGameplayTag> ResolvedRequiredEventTags;

	TMap<FGameplayTag, TArray<TObjectPtr<UMASkillAction>>> ResolvedActionsByEvent;

	/** Step **/
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMASkillStep>> RuntimeSkillSteps;

	UPROPERTY(Transient)
	int32 CurrentStepIndex = INDEX_NONE;

	UPROPERTY(Transient)
	EMASkillStepStartMode CurrentStepStartMode = EMASkillStepStartMode::Fresh;

	UPROPERTY(Transient)
	int32 PreparedStepIndex = INDEX_NONE;

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
	void HandleCurrentStepMontageCancelled();

	UFUNCTION()
	void HandleCurrentStepMontageCompleted();

	UFUNCTION()
	void HandleCurrentStepMontageInterrupted();
};
