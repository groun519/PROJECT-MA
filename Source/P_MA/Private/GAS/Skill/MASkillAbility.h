#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"
#include "GameplayTagContainer.h"
#include "MASkillAbility.generated.h"

class UDataTable;
class UMASkillDefinition;
struct FGameplayEventData;

UCLASS()
class P_MA_API UMASkillAbility : public UMAGameplayAbility
{
	GENERATED_BODY()

	friend class UMASkillDefinition;

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
	FMASkillPayloadStore& GetPayloadStore() { return PayloadStore; }
	const FMASkillPayloadStore& GetPayloadStore() const { return PayloadStore; }
	FSkillRuntimeContext& GetRuntimeContext() { return RuntimeContext; }
	const FSkillRuntimeContext& GetRuntimeContext() const { return RuntimeContext; }
	void SetDesiredMontagePlayRate(float NewPlayRate);
	float GetDesiredMontagePlayRate() const { return DesiredMontagePlayRate; }
	bool GetSkillProgressInfo(FText& OutLabel, float& OutDuration, float& OutRemainingDuration) const;
	bool CanPlaySkillMontageLocally() const;

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
	void RegisterCancelTriggers();
	void UnregisterCancelTriggers();
	void HandleCancelTriggerTagChanged(FGameplayTag Tag, int32 NewCount);
	void CacheRuntimeSkillDefinition(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo);

	/** RuntimeContext **/
	UPROPERTY(Transient)
	FSkillRuntimeContext RuntimeContext;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillDefinition> RuntimeSkillDefinition;

	UPROPERTY(Transient)
	FMASkillPayloadStore PayloadStore;

	UPROPERTY(Transient)
	float DesiredMontagePlayRate = 1.f;

	TMap<FGameplayTag, FDelegateHandle> CancelTriggerDelegateHandles;
};
