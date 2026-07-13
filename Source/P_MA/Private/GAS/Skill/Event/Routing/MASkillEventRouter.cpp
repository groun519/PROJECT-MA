#include "GAS/Skill/Event/Routing/MASkillEventRouter.h"

#include "GAS/Skill/Addon/MASkillModuleAddonStatics.h"
#include "GAS/Skill/Addon/Event/MASkillModuleEventBindingAddon.h"
#include "GAS/Skill/Addon/Event/MASkillModuleEventSourceAddon.h"
#include "GAS/Skill/Event/Source/MASkillEventSource.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"

void UMASkillEventRouter::Refresh(const TArray<FMASkillSlotRuntimeState>& SkillSlotRuntimeStates)
{
	UMASkillManagerComponent* SkillManager = GetSkillManager();
	TMap<FGameplayTag, const UMASkillEventSource*> RequiredRoutes;

	for (const FMASkillSlotRuntimeState& SlotState : SkillSlotRuntimeStates)
	{
		UMASkillModuleInstance* AssembledModuleInstance = SlotState.AssembledModuleInstance;
		if (!AssembledModuleInstance) continue;

		if (const UMASkillModuleEventSourceAddon* EventSourceAddon =
			MASkillModuleAddonStatics::FindAddon<UMASkillModuleEventSourceAddon>(*AssembledModuleInstance))
		{
			for (const UMASkillEventSource* EventSource : EventSourceAddon->GetEventSources())
			{
				if (!EventSource || !EventSource->GetEmittedTag().IsValid()) continue;

				const FGameplayTag& EventTag = EventSource->GetEmittedTag();
				const UMASkillEventSource*& RequiredSource = RequiredRoutes.FindOrAdd(EventTag);
				if (!RequiredSource)
				{
					RequiredSource = EventSource;
					continue;
				}

				ensureMsgf(
					RequiredSource->HasSameRuntimeConfiguration(*EventSource),
					TEXT("Conflicting EventSource declarations for tag %s."),
					*EventTag.ToString());
			}
		}

		const UMASkillModuleEventBindingAddon* EventBindingAddon =
			MASkillModuleAddonStatics::FindAddon<UMASkillModuleEventBindingAddon>(*AssembledModuleInstance);
		if (!EventBindingAddon) continue;

		for (const FMASkillEventBinding& Binding : EventBindingAddon->GetEventBindings())
		{
			if (!Binding.EventTag.IsValid() || !Binding.Action) continue;

			RequiredRoutes.FindOrAdd(Binding.EventTag);
		}
	}

	for (auto It = Routes.CreateIterator(); It; ++It)
	{
		if (RequiredRoutes.Contains(It.Key())) continue;

		if (It.Value()) It.Value()->DeinitializeRuntime();
		It.RemoveCurrent();
	}

	for (const TPair<FGameplayTag, const UMASkillEventSource*>& RequiredRoute : RequiredRoutes)
	{
		const FGameplayTag& EventTag = RequiredRoute.Key;
		const UMASkillEventSource* SourceDeclaration = RequiredRoute.Value;
		TObjectPtr<UMASkillEventSource>& RuntimeSource = Routes.FindOrAdd(EventTag);

		if (UMASkillEventSource* ExistingSource = RuntimeSource)
		{
			if (SourceDeclaration
				&& SourceDeclaration->RequiresRuntimeInstance()
				&& ExistingSource->HasSameRuntimeConfiguration(*SourceDeclaration))
			{
				continue;
			}

			ExistingSource->DeinitializeRuntime();
			RuntimeSource = nullptr;
		}

		if (!SourceDeclaration || !SourceDeclaration->RequiresRuntimeInstance()) continue;

		RuntimeSource = DuplicateObject<UMASkillEventSource>(SourceDeclaration, this);
		check(RuntimeSource);
		RuntimeSource->InitializeRuntime(SkillManager);
	}
}

void UMASkillEventRouter::Clear()
{
	for (const TPair<FGameplayTag, TObjectPtr<UMASkillEventSource>>& RoutePair : Routes)
	{
		if (RoutePair.Value) RoutePair.Value->DeinitializeRuntime();
	}
	Routes.Reset();
}

UMASkillManagerComponent* UMASkillEventRouter::GetSkillManager() const
{
	return CastChecked<UMASkillManagerComponent>(GetOuter());
}
