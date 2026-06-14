#include "GAS/Skill/Action/MASkillAction_MeleeOverlapHelper.h"

#include "Engine/HitResult.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/Payload/MASkillPayloadAccessor.h"

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
		? OwnerAbility.GetHitResultFromVirtualSocketTargetData(*TargetData, TargetRelationMask)
		: TArray<FHitResult>();
}

FVector MASkillActionMeleeOverlap::ResolveStatusEffectCenterPoint(
	UMASkillAbility& OwnerAbility,
	const FMASkillEvent& Event)
{
	const FGameplayAbilityTargetDataHandle* TargetData = Event.GetTargetData();
	if (TargetData && TargetData->Num() > 0 && TargetData->Data[0].IsValid())
	{
		return TargetData->Data[0]->GetOrigin().GetTranslation();
	}

	if (const AActor* AvatarActor = OwnerAbility.GetAvatarActorFromActorInfo())
	{
		return AvatarActor->GetActorLocation();
	}

	return FVector::ZeroVector;
}
