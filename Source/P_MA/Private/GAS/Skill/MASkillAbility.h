#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GAS/Skill/Input/MASkillFlowPart.h"
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
	UMASkillFlowPart* GetCurrentRuntimeFlowPart() const;
	FMASkillPayloadStore& GetPayloadStore() { return PayloadStore; }
	const FMASkillPayloadStore& GetPayloadStore() const { return PayloadStore; }
	void SetDesiredMontagePlayRate(float NewPlayRate);
	float GetDesiredMontagePlayRate() const { return DesiredMontagePlayRate; }
	void MultiplyFinalDamageMultiplier(float Multiplier) { RuntimeContext.MultiplyFinalDamageMultiplier(Multiplier); }
	void CompleteCurrentFlow(float MontageBlendOutTime = 0.f);
	bool PrepareNextFlowMontage(float PreviewBlendInTime);
	bool ActivatePreparedNextFlow();

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
	void RegisterAnimationOwner(UAnimSequenceBase* Animation);
	void UnregisterAnimationOwner(UAnimSequenceBase* Animation);
	void ResolvePayloads();
	void ResolveEventActions();
	void ResetResolvedData();
	void AddResolvedEventAction(const FGameplayTag& EventTag, UMASkillAction* Action);
	void ResolveActionsForEvent(const FGameplayTag& EventTag, TArray<UMASkillAction*>& OutActions) const;
	void HandlePreparedMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	void HandlePreparedMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void CacheRuntimeSkillDefinition(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo);
	int32 ResolveNextMontageFlowIndex(int32 CurrentIndex) const;

	/** RuntimeContext **/
	UPROPERTY(Transient)
	FSkillRuntimeContext RuntimeContext;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillDefinition> RuntimeSkillDefinition;

	UPROPERTY(Transient)
	FMASkillPayloadStore PayloadStore;

	TSet<FGameplayTag> ResolvedRequiredEventTags;

	TMap<FGameplayTag, TArray<TObjectPtr<UMASkillAction>>> ResolvedActionsByEvent;

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
