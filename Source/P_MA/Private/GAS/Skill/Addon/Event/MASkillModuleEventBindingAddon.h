#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Addon/MASkillModuleAddon.h"
#include "GAS/Skill/Event/Binding/MASkillEventBinding.h"
#include "MASkillModuleEventBindingAddon.generated.h"

/** Declares actions triggered by routed skill events. */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillModuleEventBindingAddon : public UMASkillModuleAddon
{
	GENERATED_BODY()

public:
	const TArray<FMASkillEventBinding>& GetEventBindings() const { return EventBindings; }

private:
	UPROPERTY(EditDefaultsOnly, Category="Event Binding")
	TArray<FMASkillEventBinding> EventBindings;

	friend struct FMASkillEventBindingAssembler;
};
