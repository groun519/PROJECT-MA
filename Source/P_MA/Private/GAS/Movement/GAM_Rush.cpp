// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Movement/GAM_Rush.h"
#include "AbilitySystemComponent.h"
#include "Player/MAPlayerCharacter.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"


UGAM_Rush::UGAM_Rush()
{
	
}

void UGAM_Rush::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{

	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	AMAPlayerCharacter* Character = Cast<AMAPlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
	if (Character)
	{
		GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(Character->RushingTag);
		bIsEnd=false;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,MontageToPlay);
	if (MontageTask)
	{
		MontageTask->OnBlendOut.AddDynamic(this, &UGAM_Rush::K2_EndAbility);
		MontageTask->OnCancelled.AddDynamic(this, &UGAM_Rush::K2_EndAbility);
		MontageTask->OnCompleted.AddDynamic(this, &UGAM_Rush::K2_EndAbility);
		MontageTask->OnInterrupted.AddDynamic(this, &UGAM_Rush::K2_EndAbility);
		MontageTask->ReadyForActivation();
	}

	WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	if (WaitInputReleaseTask)
	{
		WaitInputReleaseTask->OnRelease.AddDynamic(this, &UGAM_Rush::OnInputReleased);
		WaitInputReleaseTask->ReadyForActivation();
	}

	TimeoutTask = UAbilityTask_WaitDelay::WaitDelay(this,MaxHoldDuration);
	if (TimeoutTask)
	{
		TimeoutTask -> OnFinish.AddDynamic(this, &UGAM_Rush::OnTimeout);
		TimeoutTask -> ReadyForActivation();
	}

	AttackEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,FGameplayTag::RequestGameplayTag("Ability.Movement.Damage"));
	if (AttackEventTask)
	{
		AttackEventTask->EventReceived.AddDynamic(this, &UGAM_Rush::DoDamage);
		AttackEventTask->ReadyForActivation();
	}
}

void UGAM_Rush::OnInputPressed(float TimePressed)
{
	WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	WaitInputReleaseTask->OnRelease.AddDynamic(this, &UGAM_Rush::OnInputReleased);
	WaitInputReleaseTask->ReadyForActivation();
}

void UGAM_Rush::OnInputReleased(float TimePressed)
{
	if (bIsEnd)
		return;
	bIsEnd=true;
	
	MontageToEndSection();
}

void UGAM_Rush::OnTimeout()
{
	bIsEnd=true;
	MontageToEndSection();
}

void UGAM_Rush::MontageToEndSection()
{
	AMAPlayerCharacter* Character = Cast<AMAPlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
	if (Character)
	{
		GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(Character->RushingTag);
		Character->GetMesh()->GetAnimInstance()->Montage_JumpToSection("End",MontageToPlay);
	}
}

