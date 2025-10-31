// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Golem/GA_GroundAttack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "AbilitySystemGlobals.h"

UGA_GroundAttack::UGA_GroundAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_GroundAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,	const FGameplayAbilityActorInfo* ActorInfo,	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character || !GroundAttackMontage)
	{
		K2_EndAbility();
		return;
	}
	
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, GroundAttackMontage, 1.f);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_GroundAttack::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_GroundAttack::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_GroundAttack::OnMontageCompleted);
	MontageTask->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* WaitEndEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(TEXT("Ability.Combo.Change.End")));
	WaitEndEvent->EventReceived.AddDynamic(this, &UGA_GroundAttack::OnEndEventReceived);
	WaitEndEvent->ReadyForActivation();

	if (K2_HasAuthority())
	{
		UAbilityTask_WaitGameplayEvent* WaitTargetEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetTargetEventTag());
		WaitTargetEventTask->EventReceived.AddDynamic(this, &UGA_GroundAttack::HitTarget);
		WaitTargetEventTask->ReadyForActivation();
	}
}

void UGA_GroundAttack::OnEndEventReceived(FGameplayEventData Payload)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_GroundAttack::HitTarget(FGameplayEventData Data)
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

TSubclassOf<UGameplayEffect> UGA_GroundAttack::GetDamageEffect() const
{
	return DamageEffect;
}

void UGA_GroundAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_GroundAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	IgnoreTargets.Empty();
}

FGameplayTag UGA_GroundAttack::GetTargetEventTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Combo.Damage");
}
