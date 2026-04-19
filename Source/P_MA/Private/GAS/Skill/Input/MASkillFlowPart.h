#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "MASkillFlowPart.generated.h"

class UAnimMontage;
class UMASkillAbility;
struct FGameplayEventData;

UENUM()
enum class EMASkillFlowStartMode : uint8
{
	Fresh,
	Prepared
};

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillFlowPart : public UObject
{
	GENERATED_BODY()

public:
	virtual void InitializeFlow(UMASkillAbility* SkillAbility, int32 InFlowIndex, int32 InNextFlowIndex, int32 InNextMontageFlowIndex)
	{
		OwnerSkillAbility = SkillAbility;
		FlowIndex = InFlowIndex;
		NextFlowIndex = InNextFlowIndex;
		NextMontageFlowIndex = InNextMontageFlowIndex;
	}

	virtual void StartFlow(UMASkillAbility* SkillAbility, EMASkillFlowStartMode /*StartMode*/)
		{ OwnerSkillAbility = SkillAbility; }

	virtual void StopFlow()
		{ OwnerSkillAbility = nullptr; }

	virtual UAnimMontage* ResolveFlowMontage() const { return FlowMontage; }
	int32 GetFlowIndex() const { return FlowIndex; }
	int32 GetNextFlowIndex() const { return NextFlowIndex; }
	int32 GetNextMontageFlowIndex() const { return NextMontageFlowIndex; }
	virtual bool ShouldAutoAdvanceOnMontageCompleted() const { return true; }
	virtual void CollectRequiredEventTags(TSet<FGameplayTag>& OutTags) const {}
	virtual void HandleRuntimeEvent(const FGameplayEventData& Payload) {}

protected:
	UMASkillAbility* GetOwnerSkillAbility() const { return OwnerSkillAbility; }

	UPROPERTY(EditDefaultsOnly, Category="Flow")
	TObjectPtr<UAnimMontage> FlowMontage;

	// TODO: If flow variants grow and repeat the same task-owner usage patterns, replace this with a narrower flow task-owner interface.
	UPROPERTY(Transient)
	TObjectPtr<UMASkillAbility> OwnerSkillAbility;

	UPROPERTY(Transient)
	int32 FlowIndex = INDEX_NONE;

	UPROPERTY(Transient)
	int32 NextFlowIndex = INDEX_NONE;

	UPROPERTY(Transient)
	int32 NextMontageFlowIndex = INDEX_NONE;
};
