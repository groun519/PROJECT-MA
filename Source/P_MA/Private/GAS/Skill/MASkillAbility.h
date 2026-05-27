#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/MAGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "MASkillAbility.generated.h"

class UDataTable;
class UAbilityTask_WaitGameplayEvent;
class UMASkillDefinition;
class UMASkillEventSource;
class UMASkillGenericDataAsset;
class UMASkillModuleInstance;
class UMASkillStepManager;
struct FGameplayEventData;
struct FMASkillPayloadStore;

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
	const UDataTable* GetElementalDataTable() const;
	const UDataTable* GetOverlapDecalDataTable() const;
	FMASkillPayloadStore* GetModulePayloadStore(UMASkillModuleInstance* BindingScope) const;
	FMASkillPayloadStore& GetAssembledModulePayloadStore();
	UFUNCTION()
	void HandleExternalGameplayEvent(FGameplayEventData EventData);
	void ExecuteScopedGameplayEvent(UMASkillModuleInstance* EventScope, FGameplayEventData EventData, UMASkillModuleInstance* BindingScope);
	void SendSkillGameplayEvent(const FGameplayEventData& EventData, UMASkillModuleInstance* BindingScope = nullptr);
	const UMASkillDefinition* GetCurrentSkillDefinition() const;
	void UpdateCurrentSkillModuleInstance(UMASkillModuleInstance* SourceSkillModuleInstance);
	UMASkillModuleInstance* GetCurrentSkillModuleInstance() const { return CurrentSkillModuleInstance; }
	UMASkillStepManager* GetStepManager() const { return StepManager; }
	UMASkillModuleInstance* GetCurrentBindingScope() const;
	FMASkillAbilityLifecycleDelegate& OnSkillActivated() { return SkillActivatedDelegate; }
	FMASkillAbilityLifecycleDelegate& OnSkillDeactivated() { return SkillDeactivatedDelegate; }
	void EndSkill() { K2_EndAbility(); }
	bool CanPlaySkillMontageLocally() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Cancel", meta=(Categories="State,Effect"))
	// TODO: Move cancel trigger registration to UMAGameplayAbility after the remaining legacy skill-specific paths are removed.
	FGameplayTagContainer CancelTriggerTags;

private:
	const UMASkillGenericDataAsset* GetGenericSkillDataAsset() const;
	void ApplyCurrentSkillModuleInstance(UMASkillModuleInstance* SourceSkillModuleInstance);
	void RegisterCancelTriggers();
	void UnregisterCancelTriggers();
	void HandleCancelTriggerTagChanged(FGameplayTag Tag, int32 NewCount);
	void BindGameplayEvents();
	void UnbindGameplayEvents();
	void EnsureEventSources();
	void EnsureStepManager();

	UPROPERTY(Transient)
	TObjectPtr<UMASkillModuleInstance> CurrentSkillModuleInstance;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillModuleInstance> PendingSkillModuleInstance;

	bool bHasPendingSkillModuleInstanceUpdate = false;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UAbilityTask_WaitGameplayEvent>> EventTasks;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillStepManager> StepManager;

	FMASkillAbilityLifecycleDelegate SkillActivatedDelegate;
	FMASkillAbilityLifecycleDelegate SkillDeactivatedDelegate;
	TMap<FGameplayTag, FDelegateHandle> CancelTriggerDelegateHandles;
};
