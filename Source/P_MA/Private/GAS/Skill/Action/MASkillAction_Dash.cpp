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

void UMASkillAction_Dash::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Ability && Scopes);
	if (!Owner.HasAuthority()) return;

	FVector DashVelocity = Owner.GetActorForwardVector().GetSafeNormal2D() * DashSpeed;
	if (DashVelocity.IsNearlyZero()) return;
	if (DashDuration <= 0.f) return;

	const FGameplayTag DashImpulseTag = GetDashImpulseTag();
	if (AMACharacter* OwnerCharacter = Cast<AMACharacter>(&Owner))
	{
		if (UMAImpulseComponent* ImpulseComponent = OwnerCharacter->GetImpulseComponent())
		{
			ImpulseComponent->ApplyActionImpulseVelocity(Ability, DashImpulseTag, DashVelocity, DashDuration, true, Scopes->Module);
		}
	}
}
