#include "GAS/Skill/Action/MASkillAction_ApplyOverlayToSelf.h"

#include "Character/MAOverlayComponent.h"
#include "GAS/Skill/MASkillAbility.h"

void UMASkillAction_ApplyOverlayToSelf::Execute(
	UMASkillAbility& OwnerAbility,
	const FGameplayEventData&,
	const FMASkillEventScopes&)
{
	AActor* AvatarActor = OwnerAbility.GetAvatarActorFromActorInfo();
	if (!AvatarActor || AvatarActor->GetNetMode() == NM_DedicatedServer) return;

	if (UMAOverlayComponent* OverlayComponent = AvatarActor->FindComponentByClass<UMAOverlayComponent>())
	{
		OverlayComponent->ApplyTimedOverlay(BaseColor, Alpha, Duration);
	}
}
