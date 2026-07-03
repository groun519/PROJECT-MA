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
	UMASkillAbility& OwnerAbility,
	const FMASkillEvent& Event,
	const FMASkillScopes& Scopes)
{
	const FMASkillPayloadAccessor Payloads = Event.GetPayloadAccess(Scopes);
	const FMASkillDamageConfig DamageConfig = MASkillActionMeleeOverlap::ResolveDamageConfig(Payloads, DamagePayloadTag);
	if (const FGameplayAbilityTargetDataHandle* TargetData = Event.GetTargetData())
	{
		if (const FMASkillWorldAreaShape* Area = MASkillAreaStatics::FindWorldShape(*TargetData))
		{
			MASkillAreaDecalStatics::SpawnImpact(OwnerAbility, *Area, DamageConfig.DamageTypeTag);
		}
	}

	if (!OwnerAbility.K2_HasAuthority()) return;
	if (!Payloads.IsValid()) return;

	const FResolvedSkillDamage ResolvedDamage = MASkillDamageResolver::Resolve(OwnerAbility, DamageConfig, Payloads);
	const TArray<FHitResult> HitResults = MASkillActionMeleeOverlap::ResolveHitResultsFromEvent(OwnerAbility, Event, ResolvedDamage.TargetRelationMask);
	const FVector StatusEffectCenterPoint = MASkillActionMeleeOverlap::ResolveStatusEffectCenterPoint(OwnerAbility, Event);
	MASkillDamageApplicator::ApplyHitResults(OwnerAbility, Scopes, HitResults, ResolvedDamage, StatusEffectCenterPoint);
}
