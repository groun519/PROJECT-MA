#include "GAS/Skill/Action/MASkillAction_Dash.h"

#include "Character/MACharacter.h"
#include "Character/MAImpulseComponent.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"

namespace
{
	FGameplayTag GetDashImpulseTag()
	{
		return FGameplayTag::RequestGameplayTag(TEXT("Ability.Movement.Dash"));
	}
}

void UMASkillAction_Dash::Execute(UMASkillAbility& OwnerAbility, FSkillRuntimeContext&, FMASkillPayloadStore& PayloadStore, const FGameplayEventData&)
{
	(void)PayloadStore;

	if (!OwnerAbility.K2_HasAuthority()) return;

	AActor* AvatarActor = OwnerAbility.GetAvatarActorFromActorInfo();
	if (!AvatarActor) return;

	FVector DashVelocity = AvatarActor->GetActorForwardVector().GetSafeNormal2D() * DashSpeed;
	if (DashVelocity.IsNearlyZero()) return;
	if (DashDuration <= 0.f) return;

	const FGameplayTag DashImpulseTag = GetDashImpulseTag();
	if (AMACharacter* OwnerCharacter = Cast<AMACharacter>(AvatarActor))
	{
		if (UMAImpulseComponent* ImpulseComponent = OwnerCharacter->GetImpulseComponent())
		{
			ImpulseComponent->ApplyActionImpulseVelocity(&OwnerAbility, DashImpulseTag, DashVelocity, DashDuration);
		}
	}
}
