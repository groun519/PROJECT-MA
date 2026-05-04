#include "GAS/Skill/Action/MASkillAction_MeleeOverlapHelper.h"

#include "Engine/HitResult.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"

FMASkillDamageConfig MASkillActionMeleeOverlap::ResolveDamageConfig(const FMASkillPayloadStore& PayloadStore, const FGameplayTag& DamagePayloadTag)
{
	FMASkillDamageConfig DamageConfig;
	PayloadStore.TryGetStruct(DamagePayloadTag, DamageConfig);
	return DamageConfig;
}

TArray<FHitResult> MASkillActionMeleeOverlap::ResolveHitResultsFromPayload(
	UMASkillAbility& OwnerAbility,
	const FGameplayEventData& Payload,
	int32 TargetRelationMask)
{
	return OwnerAbility.GetHitResultFromVirtualSocketTargetData(Payload.TargetData, TargetRelationMask);
}

FVector MASkillActionMeleeOverlap::ResolveStatusEffectCenterPoint(
	UMASkillAbility& OwnerAbility,
	const FGameplayEventData& Payload)
{
	if (Payload.TargetData.Num() > 0 && Payload.TargetData.Data[0].IsValid())
	{
		return Payload.TargetData.Data[0]->GetOrigin().GetTranslation();
	}

	if (const AActor* AvatarActor = OwnerAbility.GetAvatarActorFromActorInfo())
	{
		return AvatarActor->GetActorLocation();
	}

	return FVector::ZeroVector;
}
