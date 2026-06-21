#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/Binding/MASkillEventBinding.h"
#include "UObject/Object.h"
#include "MASkillEventDispatcher.generated.h"

class UMASkillAbility;
struct FMASkillEvent;
struct FMASkillSlotRuntimeState;

DECLARE_MULTICAST_DELEGATE_OneParam(FMASkillEventEvaluatedSignature, const FMASkillEvent&);

USTRUCT()
struct FMASkillRegisteredEventBinding
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FMASkillEventBinding Binding;

	UPROPERTY(Transient)
	TWeakObjectPtr<UMASkillAbility> ExecutorAbility;
};

USTRUCT()
struct FMASkillRegisteredEventBindings
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TArray<FMASkillRegisteredEventBinding> Values;
};

UCLASS()
class P_MA_API UMASkillEventDispatcher : public UObject
{
	GENERATED_BODY()

public:
	void Refresh(const TArray<FMASkillSlotRuntimeState>& SkillSlotRuntimeStates);
	void Dispatch(const FMASkillEvent& Event, UMASkillAbility* ExecutorAbility);
	void DispatchGroup(TConstArrayView<FMASkillEvent> Events, UMASkillAbility* ExecutorAbility);
	void AddEventEvaluatedListener(
		FGameplayTag EventTag,
		const FMASkillEventEvaluatedSignature::FDelegate& Listener);
	void Clear();

private:
	UPROPERTY(Transient)
	TMap<FGameplayTag, FMASkillRegisteredEventBindings> BindingsByEventTag;

	TMap<FGameplayTag, FMASkillEventEvaluatedSignature> EventEvaluatedDelegates;
};
