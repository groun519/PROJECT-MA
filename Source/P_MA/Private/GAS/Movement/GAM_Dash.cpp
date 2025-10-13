// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Movement/GAM_Dash.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimNotify_SendNewPlayerTrans.h"
#include "GameFramework/Character.h"
#include "GAS/MAAbilitySystemStatics.h"


class UAbilityTask_PlayMontageAndWait;

UGAM_Dash::UGAM_Dash()
{
	RotationLock = UMAAbilitySystemStatics::GetRotationLockTag();
}

void UGAM_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
		return;
	}
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None,MontageToPlay);
	if (MontageTask)
	{
		MontageTask->OnBlendOut.AddDynamic(this, &UGAM_Dash::K2_EndAbility);
		MontageTask->OnCancelled.AddDynamic(this, &UGAM_Dash::K2_EndAbility);
		MontageTask->OnInterrupted.AddDynamic(this, &UGAM_Dash::K2_EndAbility);
		MontageTask->OnCompleted.AddDynamic(this, &UGAM_Dash::K2_EndAbility);
		MontageTask->ReadyForActivation();
	}

	EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,FGameplayTag::RequestGameplayTag("Ability.Movement.Dash.Start"));
	if (EventTask)
	{
		EventTask->EventReceived.AddDynamic(this, &UGAM_Dash::OnGameplayEventReceived);
		EventTask->ReadyForActivation();
	}

	AttackEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,UMAAbilitySystemStatics::GetMontageDamageTag());
	if (AttackEventTask)
	{
		AttackEventTask->EventReceived.AddDynamic(this, &UGAM_Dash::DoDamage);
		AttackEventTask->ReadyForActivation();
	}
}


void UGAM_Dash::OnGameplayEventReceived(FGameplayEventData Data)
{
	if (Data.TargetData.Num() >0)
	{
		const FGameplayAbilityTargetData* TargetData = Data.TargetData.Get(0);
		if (TargetData && TargetData->GetScriptStruct()->IsChildOf(FDashData::StaticStruct()))
		{
			const FDashData* DashData = static_cast<const FDashData*>(TargetData);

			const float ReceivedDashForce = DashData->DashForce;

			ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
			if (Character)
			{
				FVector LaunchVelocity = Character->GetActorForwardVector() * ReceivedDashForce;
				LaunchVelocity.Z += UpLaunchForce;
				Character->LaunchCharacter(LaunchVelocity, true,true);
			}
		}
	}
}


void UGAM_Dash::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
