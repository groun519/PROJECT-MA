#include "GAS/Skill/Event/Routing/MASkillEventRoutingStatics.h"

#include "GAS/Skill/Event/Dispatch/MASkillEventDispatcher.h"
#include "GAS/Skill/Event/Routing/MASkillEventRouter.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/MASkillManagerComponent.h"

bool UMASkillEventRoutingStatics::TryNotifySkillEvent(
	UMASkillAbility* ExecutorAbility,
	FMASkillEvent Event,
	UMASkillModuleInstance* ModuleScope)
{
	if (!ExecutorAbility) return false;

	if (!Event.SourceScopes.Module) Event.SourceScopes.Module = ModuleScope;
	if (!Event.SourceScopes.Skill) Event.SourceScopes.Skill = ExecutorAbility->GetCurrentSkillModuleInstance();
	return TryRouteEvent(ExecutorAbility->GetSkillManagerComponent(), Event, ExecutorAbility);
}

bool UMASkillEventRoutingStatics::TryNotifySkillEvent(
	UMASkillAbility* ExecutorAbility,
	FGameplayTag EventTag,
	const FMASkillScopes& SourceScopes)
{
	return TryNotifySkillEvent(ExecutorAbility, FMASkillEvent(EventTag, SourceScopes));
}

bool UMASkillEventRoutingStatics::TryNotifySkillEventGroup(
	UMASkillAbility* ExecutorAbility,
	TArray<FMASkillEvent> Events,
	UMASkillModuleInstance* ModuleScope)
{
	if (!ExecutorAbility || Events.IsEmpty()) return false;

	UMASkillModuleInstance* SkillScope = ExecutorAbility->GetCurrentSkillModuleInstance();
	for (FMASkillEvent& Event : Events)
	{
		if (!Event.SourceScopes.Module) Event.SourceScopes.Module = ModuleScope;
		if (!Event.SourceScopes.Skill) Event.SourceScopes.Skill = SkillScope;
	}
	return TryRouteEventGroup(ExecutorAbility->GetSkillManagerComponent(), MoveTemp(Events), ExecutorAbility);
}

bool UMASkillEventRoutingStatics::TryNotifyGlobalEvent(
	UMASkillManagerComponent* SkillManager,
	FMASkillEvent Event)
{
	return TryRouteEvent(SkillManager, Event, nullptr);
}

bool UMASkillEventRoutingStatics::TryRouteEvent(
	UMASkillManagerComponent* SkillManager,
	const FMASkillEvent& Event,
	UMASkillAbility* ExecutorAbility)
{
	if (!SkillManager) return false;

	UMASkillEventRouter* Router = SkillManager->GetEventRouter();
	UMASkillEventDispatcher* Dispatcher = SkillManager->GetEventDispatcher();
	if (!Router || !Dispatcher || !Router->CanRoute(Event.Tag)) return false;

	Dispatcher->Dispatch(Event, ExecutorAbility);
	return true;
}

bool UMASkillEventRoutingStatics::TryRouteEventGroup(
	UMASkillManagerComponent* SkillManager,
	TArray<FMASkillEvent> Events,
	UMASkillAbility* ExecutorAbility)
{
	if (!SkillManager || Events.IsEmpty()) return false;

	UMASkillEventRouter* Router = SkillManager->GetEventRouter();
	UMASkillEventDispatcher* Dispatcher = SkillManager->GetEventDispatcher();
	if (!Router || !Dispatcher || !Router->CanRoute(Events[0].Tag)) return false;

	Dispatcher->DispatchGroup(Events, ExecutorAbility);
	return true;
}
