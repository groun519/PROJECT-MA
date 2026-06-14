#include "GAS/Skill/Event/Routing/MASkillEventRoutingStatics.h"

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
	return TryRoute(ExecutorAbility->GetSkillManagerComponent(), MoveTemp(Event), ExecutorAbility);
}

bool UMASkillEventRoutingStatics::TryNotifySkillEvent(
	UMASkillAbility* ExecutorAbility,
	FGameplayTag EventTag,
	const FMASkillScopes& SourceScopes)
{
	return TryNotifySkillEvent(ExecutorAbility, FMASkillEvent(EventTag, SourceScopes));
}

bool UMASkillEventRoutingStatics::TryNotifyGlobalEvent(
	UMASkillManagerComponent* SkillManager,
	FMASkillEvent Event)
{
	return TryRoute(SkillManager, MoveTemp(Event), nullptr);
}

bool UMASkillEventRoutingStatics::TryRoute(
	UMASkillManagerComponent* SkillManager,
	FMASkillEvent Event,
	UMASkillAbility* ExecutorAbility)
{
	if (!SkillManager) return false;

	return SkillManager->TryRouteEvent(MoveTemp(Event), ExecutorAbility);
}
