#include "GAS/Skill/Input/MASkillFlowPart.h"

#include "GAS/Skill/MASkillAbility.h"

namespace
{
int32 ResolveNextMontageFlowIndex(const TArray<TObjectPtr<UMASkillFlowPart>>& RuntimeFlowParts, int32 CurrentIndex)
{
	for (int32 FlowIndex = CurrentIndex + 1; FlowIndex < RuntimeFlowParts.Num(); ++FlowIndex)
	{
		const UMASkillFlowPart* RuntimeFlowPart = RuntimeFlowParts[FlowIndex];
		if (RuntimeFlowPart && RuntimeFlowPart->ResolveFlowMontage())
		{
			return FlowIndex;
		}
	}

	return INDEX_NONE;
}
}

void UMASkillFlowPart::CreateRuntimeParts(UMASkillAbility* SkillAbility,
	const TArray<TObjectPtr<UMASkillFlowPart>>& FlowPartTemplates,
	TArray<TObjectPtr<UMASkillFlowPart>>& OutRuntimeFlowParts)
{
	OutRuntimeFlowParts.Reset();
	if (!SkillAbility) return;

	for (UMASkillFlowPart* FlowPartTemplate : FlowPartTemplates)
	{
		if (!FlowPartTemplate) continue;

		UMASkillFlowPart* RuntimeFlowPart = DuplicateObject<UMASkillFlowPart>(FlowPartTemplate, SkillAbility);
		if (!RuntimeFlowPart) continue;

		OutRuntimeFlowParts.Add(RuntimeFlowPart);
	}

	for (int32 FlowIndex = 0; FlowIndex < OutRuntimeFlowParts.Num(); ++FlowIndex)
	{
		UMASkillFlowPart* RuntimeFlowPart = OutRuntimeFlowParts[FlowIndex];
		if (!RuntimeFlowPart) continue;

		const int32 NextFlowIndex = OutRuntimeFlowParts.IsValidIndex(FlowIndex + 1) ? FlowIndex + 1 : INDEX_NONE;
		const int32 NextMontageFlowIndex = ResolveNextMontageFlowIndex(OutRuntimeFlowParts, FlowIndex);
		RuntimeFlowPart->InitializeFlow(SkillAbility, FlowIndex, NextFlowIndex, NextMontageFlowIndex);
	}
}

void UMASkillFlowPart::CollectCurrentRequiredEventTags(const TArray<TObjectPtr<UMASkillFlowPart>>& RuntimeFlowParts,
	int32 CurrentFlowIndex, TSet<FGameplayTag>& OutTags)
{
	if (!RuntimeFlowParts.IsValidIndex(CurrentFlowIndex)) return;

	const UMASkillFlowPart* CurrentFlowPart = RuntimeFlowParts[CurrentFlowIndex];
	if (!CurrentFlowPart) return;

	CurrentFlowPart->CollectRequiredEventTags(OutTags);
}
