#include "GAS/Skill/Action/MASkillAction_MeleeOverlapShape.h"

#include "GAS/Skill/Action/MASkillAction_MeleeOverlapHelper.h"
#include "GAS/Skill/Area/MASkillAreaStatics.h"
#include "GAS/Skill/Damage/MASkillDamageApplicator.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/MASkillAbility.h"

UMASkillAction_MeleeOverlapShape::UMASkillAction_MeleeOverlapShape()
{
	Config.Shape = EMASkillAreaShape::Circle;
}

void UMASkillAction_MeleeOverlapShape::Execute(
	UMASkillAbility& OwnerAbility,
	const FMASkillEvent& Event,
	const FMASkillScopes& Scopes)
{
	AActor* AvatarActor = OwnerAbility.GetAvatarActorFromActorInfo();
	if (!AvatarActor) return;

	const FMASkillPayloadAccessor Payloads = Event.GetPayloadAccess(Scopes);
	const FMASkillWorldAreaShape Area = Config.ResolveWorld(
		AvatarActor->GetActorTransform(),
		MASkillAreaStatics::ResolveAreaScale(Payloads));
	const FMASkillDamageConfig DamageConfig = MASkillActionMeleeOverlap::ResolveDamageConfig(Payloads, DamagePayloadTag);
	MASkillDamageApplicator::ApplyArea(OwnerAbility, Scopes, Area, DamageConfig, Payloads);
}
