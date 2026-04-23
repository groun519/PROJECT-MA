#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GameplayTagContainer.h"
#include "MASkillAbility.generated.h"

class UDataTable;
class UAbilityTask_WaitGameplayEvent;
class UMASkillDefinition;
class UMASkillEventSource;
class UMASkillStepManager;
struct FGameplayEventData;

DECLARE_MULTICAST_DELEGATE(FMASkillAbilityLifecycleDelegate);

UCLASS()
class P_MA_API UMASkillAbility : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	UMASkillAbility();
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	const FGameplayTag& GetElementalTag() const;
	const UDataTable* GetElementalDataTable() const { return ElementalDataTable; }
	const UDataTable* GetOverlapDecalDataTable() const { return OverlapDecalDataTable; }
	FMASkillPayloadStore& GetPayloadStore() { return PayloadStore; }
	const FMASkillPayloadStore& GetPayloadStore() const { return PayloadStore; }
	UFUNCTION()
	void HandleSkillGameplayEvent(FGameplayEventData Payload);
	const UMASkillDefinition* GetCurrentSkillDefinition() const { return CurrentSkillDefinition; }
	void UpdateCurrentSkillDefinition(UMASkillDefinition* SourceSkillDefinition);
	UMASkillStepManager* GetStepManager() const { return StepManager; }
	FMASkillAbilityLifecycleDelegate& OnSkillActivated() { return SkillActivatedDelegate; }
	FMASkillAbilityLifecycleDelegate& OnSkillDeactivated() { return SkillDeactivatedDelegate; }
	void EndSkill() { K2_EndAbility(); }
	bool CanPlaySkillMontageLocally() const;

protected:
	/** Definition DataAsset **/
	// TODO: Temporary editor-assigned test input. Remove after the merged current-definition pipeline fully replaces direct DA assignment.
	UPROPERTY(EditDefaultsOnly, Category="Definition")
	TObjectPtr<UMASkillDefinition> SkillDefinition;

	// TODO: Move this kind of shared lookup data into a common subsystem once the
	// skill runtime starts depending on more global registries than elemental data.
	UPROPERTY(EditDefaultsOnly, Category="Elemental", meta=(RowType="/Script/P_MA.MAElementDataRow"))
	TObjectPtr<UDataTable> ElementalDataTable;

	UPROPERTY(EditDefaultsOnly, Category="Effect", meta=(RowType="/Script/P_MA.MAOverlapDecalDataRow"))
	TObjectPtr<UDataTable> OverlapDecalDataTable;

	UPROPERTY(EditDefaultsOnly, Category="Cancel", meta=(Categories="State,Effect"))
	// TODO: Move cancel trigger registration to UMAGameplayAbility after the remaining legacy skill-specific paths are removed.
	FGameplayTagContainer CancelTriggerTags;

private:
	void RegisterCancelTriggers();
	void UnregisterCancelTriggers();
	void HandleCancelTriggerTagChanged(FGameplayTag Tag, int32 NewCount);
	void BindGameplayEvents();
	void UnbindGameplayEvents();
	void EnsureEventSources();
	void EnsureStepManager();

	UPROPERTY(Transient)
	TObjectPtr<UMASkillDefinition> CurrentSkillDefinition;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UAbilityTask_WaitGameplayEvent>> EventTasks;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillStepManager> StepManager;

	UPROPERTY(Transient)
	FMASkillPayloadStore PayloadStore;

	FMASkillAbilityLifecycleDelegate SkillActivatedDelegate;
	FMASkillAbilityLifecycleDelegate SkillDeactivatedDelegate;
	TMap<FGameplayTag, FDelegateHandle> CancelTriggerDelegateHandles;
};
