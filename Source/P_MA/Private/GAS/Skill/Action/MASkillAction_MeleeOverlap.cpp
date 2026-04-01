#include "GAS/Skill/Action/MASkillAction_MeleeOverlap.h"

#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"

void UMASkillAction_MeleeOverlap::Execute(UMASkillAbility* SkillAbility, FSkillRuntimeContext& RuntimeContext, const FGameplayEventData& Payload)
{
	if (!SkillAbility || !SkillAbility->K2_HasAuthority())
	{
		return;
	}

	const UMASkillDefinition* SkillDefinition = SkillAbility->GetSkillDefinition();
	const TSubclassOf<UGameplayEffect> ResolvedDamageEffect = SkillDefinition ? SkillDefinition->GetDefaultDamageEffect() : nullptr;
	if (!ResolvedDamageEffect)
	{
		return;
	}

	const TArray<FHitResult> HitResults = SkillAbility->GetHitResultFromVirtualSocketTargetData(Payload.TargetData);
	const int32 AbilityLevel = SkillAbility->GetAbilityLevel(SkillAbility->GetCurrentAbilitySpecHandle(), SkillAbility->GetCurrentActorInfo());

	TSet<AActor*> HitActors;
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor || HitActors.Contains(HitActor) || RuntimeContext.IsIgnoredActor(HitActor))
		{
			continue;
		}

		SkillAbility->ApplyGameplayEffectToHitResultActor(HitResult, ResolvedDamageEffect, AbilityLevel);
		HitActors.Add(HitActor);
		RuntimeContext.AddIgnoredActor(HitActor);
	}
}
