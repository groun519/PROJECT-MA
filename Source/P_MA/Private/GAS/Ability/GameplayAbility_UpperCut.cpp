// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/GameplayAbility_UpperCut.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/MAAbilitySystemStatics.h"

UGameplayAbility_UpperCut::UGameplayAbility_UpperCut()
{
	BlockAbilitiesWithTag.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGameplayAbility_UpperCut::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayUpperCutMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, UpperCutMontage);
		PlayUpperCutMontageTask->OnBlendOut.AddDynamic(this, &UGameplayAbility_UpperCut::K2_EndAbility);
		PlayUpperCutMontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_UpperCut::K2_EndAbility);
		PlayUpperCutMontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_UpperCut::K2_EndAbility);
		PlayUpperCutMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_UpperCut::K2_EndAbility);
		PlayUpperCutMontageTask->ReadyForActivation();
	}

	UAbilityTask_WaitGameplayEvent* WaitLaunchEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetUpperCutLaunchTag());
	WaitLaunchEventTask->EventReceived.AddDynamic(this, &UGameplayAbility_UpperCut::StartLaunching);
	WaitLaunchEventTask->ReadyForActivation();
}

FGameplayTag UGameplayAbility_UpperCut::GetUpperCutLaunchTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Uppercut.Launch");
}

void UGameplayAbility_UpperCut::StartLaunching(FGameplayEventData EventData)
{
	TArray<FHitResult> HitResults = GetHitResultFromVirtualSocketTargetData(EventData.TargetData, ETeamAttitude::Hostile, ShouldDrawDebug(), true);
	
	if (K2_HasAuthority())
	{
		PushTarget(GetAvatarActorFromActorInfo(), FVector::UpVector * UpperCutLaunchSpeed);
		for (FHitResult& HitResult : HitResults)
		{
			PushTarget(HitResult.GetActor(), FVector::UpVector * UpperCutLaunchSpeed);
			ApplyGameplayEffectToHitResultActor(HitResult, LaunchDamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}
	}
}
