#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "MASkillStep.generated.h"

class UAnimMontage;
class UMASkillAbility;
struct FGameplayEventData;

UENUM()
enum class EMASkillStepStartMode : uint8
{
	Fresh,
	Prepared
};

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillStep : public UObject
{
	GENERATED_BODY()

public:
	virtual void InitializeStep(UMASkillAbility* SkillAbility, int32 InStepIndex, int32 InNextStepIndex, int32 InNextMontageStepIndex)
	{
		OwnerSkillAbility = SkillAbility;
		StepIndex = InStepIndex;
		NextStepIndex = InNextStepIndex;
		NextMontageStepIndex = InNextMontageStepIndex;
	}

	virtual void StartStep(UMASkillAbility* SkillAbility, EMASkillStepStartMode StartMode);

	virtual void StopStep();

	virtual UAnimMontage* ResolveStepMontage() const { return StepMontage; }
	virtual FName ResolveStepStartSectionName() const;
	virtual FName ResolvePreparedStepStartSectionName() const;
	int32 GetStepIndex() const { return StepIndex; }
	int32 GetNextStepIndex() const { return NextStepIndex; }
	int32 GetNextMontageStepIndex() const { return NextMontageStepIndex; }
	virtual bool ShouldAutoAdvanceOnMontageCompleted() const { return true; }
	virtual void CollectRequiredEventTags(TSet<FGameplayTag>& OutTags) const {}
	virtual void HandleRuntimeEvent(const FGameplayEventData& Payload) {}
	static void CreateRuntimeSteps(UMASkillAbility* SkillAbility,
		const TArray<TObjectPtr<UMASkillStep>>& StepTemplates,
		TArray<TObjectPtr<UMASkillStep>>& OutRuntimeSkillSteps);
	static void CollectCurrentRequiredStepEventTags(const TArray<TObjectPtr<UMASkillStep>>& RuntimeSkillSteps,
		int32 CurrentStepIndex, TSet<FGameplayTag>& OutTags);

protected:
	UMASkillAbility* GetOwnerSkillAbility() const { return OwnerSkillAbility; }

	UPROPERTY(EditDefaultsOnly, Category="Step")
	TObjectPtr<UAnimMontage> StepMontage;

	UPROPERTY(EditDefaultsOnly, Category="Step", meta=(DisplayName="SequenceSectionNameBase"))
	FName SequenceSectionNameBase = NAME_None;

	UPROPERTY(EditDefaultsOnly, Category="Step", meta=(ClampMin="0", DisplayName="MaxSectionIndex"))
	int32 MaxSequenceSectionCount = 0;

	// TODO: If step variants grow and repeat the same task-owner usage patterns, replace this with a narrower step task-owner interface.
	UPROPERTY(Transient)
	TObjectPtr<UMASkillAbility> OwnerSkillAbility;

	UPROPERTY(Transient)
	int32 StepIndex = INDEX_NONE;

	UPROPERTY(Transient)
	int32 NextStepIndex = INDEX_NONE;

	UPROPERTY(Transient)
	int32 NextMontageStepIndex = INDEX_NONE;

	UPROPERTY(Transient)
	int32 RuntimeSequenceSectionIndex = 0;

	int32 ResolveNextSequenceSectionIndex() const;
	FName MakeSequenceSectionName(int32 SectionIndex) const;
};
