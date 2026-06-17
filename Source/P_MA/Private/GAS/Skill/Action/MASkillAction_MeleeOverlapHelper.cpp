#include "GAS/Skill/Action/MASkillAction_MeleeOverlapHelper.h"

#include "Engine/HitResult.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/Payload/MASkillPayloadAccessor.h"
#include "GAS/Skill/Area/MASkillAreaStatics.h"
#include "GAS/Skill/Area/MASkillAreaTypes.h"

FMASkillDamageConfig MASkillActionMeleeOverlap::ResolveDamageConfig(const FMASkillPayloadAccessor& Payloads, const FGameplayTag& DamagePayloadTag)
{
	FMASkillDamageConfig DamageConfig;
	Payloads.TryGetStruct(DamagePayloadTag, DamageConfig);
	return DamageConfig;
}

TArray<FHitResult> MASkillActionMeleeOverlap::ResolveHitResultsFromEvent(
	UMASkillAbility& OwnerAbility,
	const FMASkillEvent& Event,
	int32 TargetRelationMask)
{
	const FGameplayAbilityTargetDataHandle* TargetData = Event.GetTargetData();
	return TargetData
		? OwnerAbility.GetHitResultsFromAreaTargetData(*TargetData, TargetRelationMask)
		: TArray<FHitResult>();
}

FVector MASkillActionMeleeOverlap::ResolveStatusEffectCenterPoint(
	UMASkillAbility& OwnerAbility,
	const FMASkillEvent& Event)
{
	const FGameplayAbilityTargetDataHandle* TargetData = Event.GetTargetData();
	if (TargetData)
	{
		if (const FMASkillWorldAreaShape* Area = MASkillAreaStatics::FindWorldShape(*TargetData))
		{
			return Area->Center;
		}
	}

	if (const AActor* AvatarActor = OwnerAbility.GetAvatarActorFromActorInfo())
	{
		return AvatarActor->GetActorLocation();
	}

	return FVector::ZeroVector;
}
