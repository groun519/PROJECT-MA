#include "GAS/Skill/Action/MASkillAction_ApplyDamageToPayloadTarget.h"

#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Damage/MASkillDamageApplicator.h"
#include "GAS/Skill/Damage/MASkillDamageResolver.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"

void UMASkillAction_ApplyDamageToPayloadTarget::Execute(
	UMASkillAbility& OwnerAbility,
	const FGameplayEventData& EventData,
	const FMASkillEventScopes& Scopes)
{
	(void)EventData;

	if (!OwnerAbility.K2_HasAuthority()) return;
	if (!Scopes.EventScope) return;

	const FMASkillPayloadStore& PayloadStore = Scopes.EventScope->GetPayloadStore();

	UObject* TargetObject = nullptr;
	if (!PayloadStore.TryGetObject(TargetPayloadTag, TargetObject)) return;

	AActor* TargetActor = Cast<AActor>(TargetObject);
	if (!TargetActor) return;

	FMASkillDamageConfig DamageConfig;
	if (!PayloadStore.TryGetStruct(DamagePayloadTag, DamageConfig)) return;

	const FResolvedSkillDamage ResolvedDamage = MASkillDamageResolver::Resolve(OwnerAbility, DamageConfig, PayloadStore);
	MASkillDamageApplicator::ApplyToTargetActor(
		OwnerAbility,
		Scopes.EventScope,
		*TargetActor,
		ResolvedDamage,
		TargetActor->GetActorLocation());
}
