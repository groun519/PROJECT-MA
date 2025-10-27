// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/SkillBehavior_ApplyEffectForward.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "MAGameplayAbility_SkillBase.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/MACharacter.h"

void USkillBehavior_ApplyEffectForward::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();

	WaitDamageEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, DamageEventTag);
	WaitDamageEventTask->EventReceived.AddDynamic(this, &USkillBehavior_ApplyEffectForward::OnDamageEventReceived);
	WaitDamageEventTask->ReadyForActivation();
}

void USkillBehavior_ApplyEffectForward::OnEndAbility_Implementation()
{
	if (WaitDamageEventTask.IsValid())
		WaitDamageEventTask->EndTask();
	
	Super::OnEndAbility_Implementation();
}

void USkillBehavior_ApplyEffectForward::OnDamageEventReceived(FGameplayEventData Payload)
{
	if (!Character || !OwningAbility)
		return;
	
	const FGameplayAbilityActivationInfo ActivationInfo = OwningAbility->GetCurrentActivationInfo();
	if (!OwningAbility->HasAuthority(&ActivationInfo))
		return;
	
	if (!DamageEffect)
		return;
	
	TArray<FHitResult> HitResults = OwningAbility->GetHitResultFromVirtualSocketTargetData(Payload.TargetData);
	OwningAbility->ApplyDamageToHitResults(HitResults, DamageEffect);
}

