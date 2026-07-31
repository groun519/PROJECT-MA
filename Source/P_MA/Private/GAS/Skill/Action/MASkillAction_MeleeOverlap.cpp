#include "GAS/Skill/Action/MASkillAction_MeleeOverlap.h"

#include "GAS/Skill/Action/MASkillAction_MeleeOverlapHelper.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Damage/MASkillDamageApplicator.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/Damage/MASkillDamageResolver.h"
#include "GAS/Skill/Area/MASkillAreaStatics.h"
#include "GAS/Skill/Area/MASkillAreaTypes.h"
#include "GAS/Skill/Area/Decal/MASkillAreaDecalStatics.h"

void UMASkillAction_MeleeOverlap::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Ability && Scopes);
	const FMASkillPayloadAccessor Payloads = Event.GetPayloadAccess(*Scopes);
	const FMASkillDamageConfig DamageConfig = MASkillActionMeleeOverlap::ResolveDamageConfig(Payloads, DamagePayloadTag);
	if (const FGameplayAbilityTargetDataHandle* TargetData = Event.GetTargetData())
	{
		if (const FMASkillWorldAreaShape* Area = MASkillAreaStatics::FindWorldShape(*TargetData))
		{
			MASkillAreaDecalStatics::SpawnImpact(*Ability, *Area, DamageConfig.DamageTypeTag);
		}
	}

	if (!Owner.HasAuthority()) return;
	if (!Payloads.IsValid()) return;

	const FResolvedSkillDamage ResolvedDamage = MASkillDamageResolver::Resolve(*Ability, DamageConfig, Payloads);
	const TArray<FHitResult> HitResults = MASkillActionMeleeOverlap::ResolveHitResultsFromEvent(*Ability, Event, ResolvedDamage.TargetRelationMask);
	const FVector StatusEffectCenterPoint = MASkillActionMeleeOverlap::ResolveStatusEffectCenterPoint(*Ability, Event);
	MASkillDamageApplicator::ApplyHitResults(*Ability, *Scopes, HitResults, ResolvedDamage, StatusEffectCenterPoint);
}
