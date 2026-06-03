// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Ability/GA_MonsterDash.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameplayTagsManager.h"
#include "GameFramework/Character.h"

void UGA_MonsterDash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                      const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	IgnoreTargets.Empty();

	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	ACharacter* Monster = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Monster || !DashMontage)
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

	UAbilityTask_PlayMontageAndWait* PlayComboMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DashMontage);
	PlayComboMontageTask->OnBlendOut.AddDynamic(this, &UGA_MonsterDash::K2_EndAbility);
	PlayComboMontageTask->OnCancelled.AddDynamic(this, &UGA_MonsterDash::K2_EndAbility);
	PlayComboMontageTask->OnCompleted.AddDynamic(this, &UGA_MonsterDash::K2_EndAbility);
	PlayComboMontageTask->OnInterrupted.AddDynamic(this, &UGA_MonsterDash::K2_EndAbility);
	PlayComboMontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitComboEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(TEXT("Ability.Combo.Change.Combo02")));
	WaitComboEvent->EventReceived.AddDynamic(this, &UGA_MonsterDash::OnComboChangeEvent);
	WaitComboEvent->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* WaitDamageEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(TEXT("Ability.Combo.Damage")));
	WaitDamageEvent->EventReceived.AddDynamic(this, &UGA_MonsterDash::OnDamageEvent);
	WaitDamageEvent->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* WaitEndEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(TEXT("Monster.Ability.End")));
	WaitEndEvent->EventReceived.AddDynamic(this, &UGA_MonsterDash::OnEndEventReceived);
	WaitEndEvent->ReadyForActivation();
}


void UGA_MonsterDash::OnComboChangeEvent(FGameplayEventData Data)
{
	ACharacter* Monster = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Monster) return;

	UAnimInstance* Anim = Monster->GetMesh()->GetAnimInstance();
	if (!Anim) return;

	TArray<FName> TagParts;
	UGameplayTagsManager::Get().SplitGameplayTagFName(Data.EventTag, TagParts);

	FName NextSection = TagParts.Last();

	Anim->Montage_SetNextSection(Anim->Montage_GetCurrentSection(DashMontage),	NextSection, DashMontage);
}

void UGA_MonsterDash::OnDamageEvent(FGameplayEventData Data)
{
	IgnoreTargets.Empty();
	
	TArray<FHitResult> HitResults = GetHitResultFromVirtualSocketTargetData(Data.TargetData);

	for (const FHitResult& HitResult : HitResults)
	{
		if (IgnoreTargets.Contains(HitResult.GetActor()))
			continue;

		ApplyGameplayEffectToHitResultActor(HitResult, DamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));

		IgnoreTargets.Add(HitResult.GetActor());
	}
}

void UGA_MonsterDash::OnEndEventReceived(FGameplayEventData Data)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_MonsterDash::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	IgnoreTargets.Empty();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
