#include "GAS/Skill/Addon/Cooldown/MASkillModuleCooldownAddon.h"

#include "GAS/Skill/Event/Dispatch/MASkillEventDispatcher.h"
#include "GAS/Skill/Event/MASkillEventTypes.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"

void UMASkillModuleCooldownAddon::RegisterEventSubscriptions(
	UMASkillEventDispatcher& EventDispatcher,
	UMASkillModuleInstance& ModuleInstance,
	UMASkillModuleInstance& SkillScope) const
{
	if (DurationSeconds <= 0.f || TriggerEventTags.IsEmpty()) return;

	const TWeakObjectPtr<UMASkillModuleInstance> WeakModuleInstance = &ModuleInstance;
	const TWeakObjectPtr<UMASkillModuleInstance> WeakSkillScope = &SkillScope;
	for (const FGameplayTag& EventTag : TriggerEventTags)
	{
		EventDispatcher.Subscribe(
			EventTag,
			FMASkillEventEvaluatedSignature::FDelegate::CreateUObject(
				this,
				&UMASkillModuleCooldownAddon::HandleCooldownEvent,
				WeakModuleInstance,
				WeakSkillScope));
	}
}

void UMASkillModuleCooldownAddon::HandleCooldownEvent(
	const FMASkillEvent& Event,
	TWeakObjectPtr<UMASkillModuleInstance> ModuleInstance,
	TWeakObjectPtr<UMASkillModuleInstance> SkillScope) const
{
	UMASkillModuleInstance* Module = ModuleInstance.Get();
	if (!Module || !Module->IsActive() || Module->IsCooldownActive()) return;

	switch (BindingScope)
	{
	case EMASkillEventBindingScope::Module:
		if (Event.SourceScopes.Module != Module) return;
		break;

	case EMASkillEventBindingScope::Skill:
		if (Event.SourceScopes.Skill != SkillScope.Get()) return;
		break;

	case EMASkillEventBindingScope::Global:
		break;
	}

	Module->StartCooldown(DurationSeconds);
}
