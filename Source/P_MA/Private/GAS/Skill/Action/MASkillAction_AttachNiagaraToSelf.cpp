#include "GAS/Skill/Action/MASkillAction_AttachNiagaraToSelf.h"

#include "Character/MACharacter.h"

void UMASkillAction_AttachNiagaraToSelf::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	if (!Owner.HasAuthority() || !NiagaraSystem) return;

	AMACharacter* OwnerCharacter = Cast<AMACharacter>(&Owner);
	if (!OwnerCharacter) return;

	OwnerCharacter->Multicast_AttachNiagaraToSelf(NiagaraSystem, SocketName, LifeSpan);
}
