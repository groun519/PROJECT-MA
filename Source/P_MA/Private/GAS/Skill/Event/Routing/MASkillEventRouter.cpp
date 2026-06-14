#include "GAS/Skill/Event/Routing/MASkillEventRouter.h"

#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Event/Source/MASkillEventSource.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"

bool UMASkillEventRouter::TryRoute(FMASkillEvent Event, UMASkillAbility* ExecutorAbility)
{
	if (!Routes.Contains(Event.Tag)) return false;

	GetSkillManager()->DispatchEvent(Event, ExecutorAbility);
	return true;
}

void UMASkillEventRouter::Refresh(const TArray<FMASkillSlotRuntimeState>& SkillSlotRuntimeStates)
{
	UMASkillManagerComponent* SkillManager = GetSkillManager();
	TMap<FGameplayTag, const UMASkillEventSource*> RequiredRoutes;

	for (const FMASkillSlotRuntimeState& SlotState : SkillSlotRuntimeStates)
	{
		const UMASkillDefinition* Definition = SlotState.AssembledModuleInstance
			? SlotState.AssembledModuleInstance->GetDefinition()
			: nullptr;
		if (!Definition) continue;

		for (const UMASkillEventSource* EventSource : Definition->GetEventSources())
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

		for (const FMASkillEventBinding& EventBinding : Definition->GetEventBindings())
		{
			if (!EventBinding.EventTag.IsValid() || !EventBinding.Action) continue;

			RequiredRoutes.FindOrAdd(EventBinding.EventTag);
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
