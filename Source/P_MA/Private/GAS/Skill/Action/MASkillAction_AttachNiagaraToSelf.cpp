#include "GAS/Skill/Action/MASkillAction_AttachNiagaraToSelf.h"

#include "Character/MACharacter.h"
#include "GAS/Skill/MASkillAbility.h"

void UMASkillAction_AttachNiagaraToSelf::Execute(
	UMASkillAbility& OwnerAbility,
	const FMASkillEvent&,
	const FMASkillScopes&)
{
	if (!OwnerAbility.K2_HasAuthority() || !NiagaraSystem) return;

	AMACharacter* OwnerCharacter = Cast<AMACharacter>(OwnerAbility.GetAvatarActorFromActorInfo());
	if (!OwnerCharacter) return;

	OwnerCharacter->Multicast_AttachNiagaraToSelf(NiagaraSystem, SocketName, LifeSpan);
}
