#include "GAS/Skill/Action/MASkillAction_MeleeOverlapShape.h"

#include "GAS/Skill/Action/MASkillAction_MeleeOverlapHelper.h"
#include "GAS/Skill/Area/MASkillAreaStatics.h"
#include "GAS/Skill/Damage/MASkillDamageApplicator.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/MASkillAbility.h"

UMASkillAction_MeleeOverlapShape::UMASkillAction_MeleeOverlapShape()
{
	SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub;
	Config.Shape = EMASkillAreaShape::Circle;
}

void UMASkillAction_MeleeOverlapShape::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Ability && Scopes);

	const FMASkillPayloadAccessor Payloads = Event.GetPayloadAccess(*Scopes);
	const FMASkillWorldAreaShape Area = Config.ResolveWorld(
		Owner.GetActorTransform(),
		MASkillAreaStatics::ResolveAreaScale(
			Payloads,
			Ability->GetAbilitySystemComponentFromActorInfo()));
	const FMASkillDamageConfig DamageConfig = MASkillActionMeleeOverlap::ResolveDamageConfig(Payloads, DamagePayloadTag);
	MASkillDamageApplicator::ApplyArea(*Ability, *Scopes, Area, DamageConfig, Payloads);
}
