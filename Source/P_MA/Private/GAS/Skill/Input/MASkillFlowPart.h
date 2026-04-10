#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "MASkillFlowPart.generated.h"

class UAnimMontage;
class UMASkillAbility;
struct FGameplayEventData;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillFlowPart : public UObject
{
	GENERATED_BODY()

public:
	virtual void StartFlow(UMASkillAbility* SkillAbility)
	{
		OwnerSkillAbility = SkillAbility;
	}

	virtual void StopFlow()
	{
		OwnerSkillAbility = nullptr;
	}

	UAnimMontage* ResolveFlowMontage() const { return FlowMontage; }
	virtual void CollectRequiredEventTags(TSet<FGameplayTag>& OutTags) const {}
	virtual void HandleRuntimeEvent(const FGameplayEventData& Payload) {}

protected:
	UMASkillAbility* GetOwnerSkillAbility() const { return OwnerSkillAbility; }

	UPROPERTY(EditDefaultsOnly, Category="Flow")
	TObjectPtr<UAnimMontage> FlowMontage;

	// TODO: If flow variants grow and repeat the same task-owner usage patterns, replace this with a narrower flow task-owner interface.
	UPROPERTY(Transient)
	TObjectPtr<UMASkillAbility> OwnerSkillAbility;
};
