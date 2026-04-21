#include "GAS/Skill/Action/MASkillAction_Dash.h"

#include "Character/MACharacter.h"
#include "Character/MAImpulseComponent.h"
#include "GAS/Skill/MASkillAbility.h"

namespace
{
	FGameplayTag GetDashImpulseTag()
	{
		return FGameplayTag::RequestGameplayTag(TEXT("Ability.Movement.Dash"));
	}
}

void UMASkillAction_Dash::Execute(UMASkillAbility& OwnerAbility, const FGameplayEventData&)
{
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
