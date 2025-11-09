// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Ability/GA_SuicideAttack.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameplayTagsManager.h"
#include "GameFramework/Character.h"

void UGA_SuicideAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	IgnoreTargets.Empty();

	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	ACharacter* Monster = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Monster || !SuicideMontage)
	{
		K2_EndAbility();
		return;
	}

	UAnimInstance* Anim = Monster->GetMesh()->GetAnimInstance();
	if (!Anim)
	{
		K2_EndAbility();
		return;
	}

	UAbilityTask_PlayMontageAndWait* PlayComboMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SuicideMontage);
	PlayComboMontageTask->OnBlendOut.AddDynamic(this, &UGA_SuicideAttack::K2_EndAbility);
	PlayComboMontageTask->OnCancelled.AddDynamic(this, &UGA_SuicideAttack::K2_EndAbility);
	PlayComboMontageTask->OnCompleted.AddDynamic(this, &UGA_SuicideAttack::K2_EndAbility);
	PlayComboMontageTask->OnInterrupted.AddDynamic(this, &UGA_SuicideAttack::K2_EndAbility);
	PlayComboMontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitDamageEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(TEXT("Ability.Combo.Damage")));
	WaitDamageEvent->EventReceived.AddDynamic(this, &UGA_SuicideAttack::OnDamageEvent);
	WaitDamageEvent->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* WaitEndEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(TEXT("Monster.Ability.End")));
	WaitEndEvent->EventReceived.AddDynamic(this, &UGA_SuicideAttack::OnEndEventReceived);
	WaitEndEvent->ReadyForActivation();
}

void UGA_SuicideAttack::OnRandomDelayFinished()
{
	
}

void UGA_SuicideAttack::OnDamageEvent(FGameplayEventData Data)
{
	TArray<FHitResult> HitResults = GetHitResultFromVirtualSocketTargetData(Data.TargetData);

	for (const FHitResult& HitResult : HitResults)
	{
		if (IgnoreTargets.Contains(HitResult.GetActor()))
			continue;

		ApplyGameplayEffectToHitResultActor(HitResult, DamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));

		IgnoreTargets.Add(HitResult.GetActor());
	}
}

void UGA_SuicideAttack::OnEndEventReceived(FGameplayEventData Data)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
