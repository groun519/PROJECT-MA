// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Ability/GA_MonsterDash.h"

#include "AbilitySystemGlobals.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"

UGA_MonsterDash::UGA_MonsterDash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_MonsterDash::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character || !DashMontage)
	{
		K2_EndAbility();
		return;
	}
	
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DashMontage, 1.f);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_MonsterDash::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_MonsterDash::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_MonsterDash::OnMontageCompleted);
	MontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitEndEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(TEXT("Ability.Combo.Change.End")));
	WaitEndEvent->EventReceived.AddDynamic(this, &UGA_MonsterDash::OnEndEventReceived);
	WaitEndEvent->ReadyForActivation();

	if (K2_HasAuthority())
	{
		UAbilityTask_WaitGameplayEvent* WaitTargetEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetTargetEventTag());
		WaitTargetEventTask->EventReceived.AddDynamic(this, &UGA_MonsterDash::HitTarget);
		WaitTargetEventTask->ReadyForActivation();
	}
}

void UGA_MonsterDash::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
}

FGameplayTag UGA_MonsterDash::GetTargetEventTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Combo.Damage");
}

void UGA_MonsterDash::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_MonsterDash::OnEndEventReceived(FGameplayEventData Data)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_MonsterDash::HitTarget(FGameplayEventData Data)
{
	TArray<FHitResult> HitResults = GetHitResultFromVirtualSocketTargetData(Data.TargetData);

	if (HitResults.Num() == 0)
		return;

	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();

		if (!HitActor)
			continue;
		
		UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor);

		if (!TargetASC)
			continue;
		
		if (IgnoreTargets.Contains(HitActor))
			continue;
		
		TSubclassOf<UGameplayEffect> GameplayEffect = GetDamageEffect();
		ApplyGameplayEffectToHitResultActor(HitResult, GameplayEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));

		IgnoreTargets.Add(HitActor);
	}
}

TSubclassOf<UGameplayEffect> UGA_MonsterDash::GetDamageEffect() const
{
	return DamageEffect;
}
