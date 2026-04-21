#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GAS/Skill/Event/MASkillGameplayEventPart.h"
#include "GAS/Skill/Payload/MASkillPayloadEntry.h"
#include "GAS/Skill/Step/MASkillStep.h"
#include "GameplayTagContainer.h"
#include "MASkillDefinition.generated.h"

class UAbilityTask_WaitGameplayEvent;
class UMASkillAbility;
class UMASkillEventSource;
struct FMASkillPayloadStore;
class UMASkillAction;
struct FGameplayEventData;

UCLASS(BlueprintType)
class P_MA_API UMASkillDefinition : public UDataAsset
{
	GENERATED_BODY()

	friend class UMASkillStep;

public:
	const FGameplayTag& GetElementalTag() const { return ElementalTag; }
	void ActivateSkill(UMASkillAbility* SkillAbility);
	void DeactivateSkill();
	void HandleSkillGameplayEvent(FGameplayEventData Payload);
	void ApplyDesiredMontagePlayRate(float DesiredMontagePlayRate) const;
	bool GetSkillProgressInfo(FText& OutLabel, float& OutDuration, float& OutRemainingDuration) const;
	void ResetActionRuntimeStates();

	void ApplyPayloadsTo(FMASkillPayloadStore& PayloadStore) const
	{
		for (const FMASkillPayloadEntry& PayloadEntry : Payloads)
		{
			PayloadEntry.ApplyTo(PayloadStore);
		}
	}

private:
	void InitializeRuntimeState(UMASkillAbility* SkillAbility);
	void EnterCurrentStep();
	void RebindEventTasks();
	void ClearPreparedStepPreviews();
	UMASkillStep* GetRuntimeSkillStep(int32 StepIndex) const;
	UMASkillStep* GetCurrentRuntimeSkillStep() const;
	void EndOwningSkillAbility();

	UFUNCTION()
	void HandleBoundGameplayEvent(FGameplayEventData Payload);

	UPROPERTY(EditDefaultsOnly, Category="Elemental", meta=(Categories="Elemental"))
	FGameplayTag ElementalTag;

	/** Preferred step pipeline. Each step owns its own montage and runtime logic. **/
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Step")
	TArray<TObjectPtr<UMASkillStep>> SkillSteps;

	/** Event Source **/
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Event")
	TArray<TObjectPtr<UMASkillEventSource>> EventSources;

	/** Event **/
	UPROPERTY(EditDefaultsOnly, Category="Event")
	TArray<FMASkillGameplayEventPart> EventParts;

	UPROPERTY(EditDefaultsOnly, Category="Payload")
	TArray<FMASkillPayloadEntry> Payloads;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillAbility> OwnerSkillAbility;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UAbilityTask_WaitGameplayEvent>> EventTasks;

	UPROPERTY(Transient)
	int32 CurrentStepIndex = INDEX_NONE;

	UPROPERTY(Transient)
	EMASkillStepStartMode CurrentStepStartMode = EMASkillStepStartMode::Fresh;

	UPROPERTY(Transient)
	bool bRuntimeInitialized = false;

	TSet<FGameplayTag> ResolvedRequiredEventTags;

	TMap<FGameplayTag, TArray<TObjectPtr<UMASkillAction>>> ResolvedActionsByEvent;
};
