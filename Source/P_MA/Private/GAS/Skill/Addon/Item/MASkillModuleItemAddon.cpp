#include "GAS/Skill/Addon/Item/MASkillModuleItemAddon.h"

#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Skill/Addon/Event/MASkillModuleEventBindingAddon.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GameplayTagContainer.h"

const FGameplayTag& UMASkillModuleItemAddon::GetUseEventTag()
{
	static const FGameplayTag UseEventTag =
		FGameplayTag::RequestGameplayTag(TEXT("Event.Item.Use"));
	return UseEventTag;
}

void UMASkillModuleItemAddon::Use(
	AActor& Owner,
	const UMASkillModule& Module) const
{
	const UMASkillModuleEventBindingAddon* EventBindingAddon =
		Module.FindAddon<UMASkillModuleEventBindingAddon>();
	check(EventBindingAddon);

	FMASkillEvent Event(GetUseEventTag());
	for (const FMASkillEventBinding& Binding : EventBindingAddon->GetEventBindings())
	{
		if (Binding.EventTag != Event.Tag) continue;
		check(Binding.Action);
		Binding.Action->Execute(Owner, nullptr, Event, nullptr);
	}
}
