#include "GAS/Skill/Action/MASkillAction_ApplyOverlayToSelf.h"

#include "Character/MAOverlayComponent.h"
#include "GameFramework/Actor.h"

void UMASkillAction_ApplyOverlayToSelf::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	if (Owner.GetNetMode() == NM_DedicatedServer) return;

	if (UMAOverlayComponent* OverlayComponent = Owner.FindComponentByClass<UMAOverlayComponent>())
	{
		OverlayComponent->ApplyTimedOverlay(BaseColor, Alpha, Duration);
	}
}
