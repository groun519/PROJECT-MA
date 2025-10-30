// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/SkillBehavior_Default.h"

#include "MAGameplayAbility_SkillBase.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

void USkillBehavior_Default::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();

	if (OwningAbility->K2_HasAuthority())
	{
		WaitHitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, DamageEventTag);
		WaitHitEventTask->EventReceived.AddDynamic(this, &USkillBehavior_Default::HitTarget);
		WaitHitEventTask->ReadyForActivation();
	}
}

void USkillBehavior_Default::OnEndAbility_Implementation()
{
	if (WaitHitEventTask.IsValid())
		WaitHitEventTask->EndTask();
	Super::OnEndAbility_Implementation();
}

void USkillBehavior_Default::HitTarget(FGameplayEventData EventData)
{
	TArray<FHitResult> HitResults = OwningAbility->GetHitResultFromVirtualSocketTargetData(EventData.TargetData);
	OwningAbility->ApplyDamageToHitResults(HitResults, DamageEffect);

}
