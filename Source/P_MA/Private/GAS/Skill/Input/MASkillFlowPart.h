#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "MASkillFlowPart.generated.h"

class UMASkillAbility;
struct FSkillRuntimeContext;
struct FGameplayEventData;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillFlowPart : public UObject
{
	GENERATED_BODY()

public:
	virtual void StartFlow(UMASkillAbility* SkillAbility, FSkillRuntimeContext* InRuntimeContext)
	{
		OwnerSkillAbility = SkillAbility;
		RuntimeContext = InRuntimeContext;
	}

	virtual void StopFlow()
	{
		OwnerSkillAbility = nullptr;
		RuntimeContext = nullptr;
	}

	virtual void CollectRequiredEventTags(TSet<FGameplayTag>& OutTags) const {}
	virtual void HandleRuntimeEvent(const FGameplayEventData& Payload) {}

protected:
	UMASkillAbility* GetOwnerSkillAbility() const { return OwnerSkillAbility; }
	FSkillRuntimeContext* GetRuntimeContext() const { return RuntimeContext; }

	// TODO: If flow variants grow and repeat the same task-owner usage patterns, replace this with a narrower flow task-owner interface.
	UPROPERTY(Transient)
	TObjectPtr<UMASkillAbility> OwnerSkillAbility;

	FSkillRuntimeContext* RuntimeContext = nullptr;
};
