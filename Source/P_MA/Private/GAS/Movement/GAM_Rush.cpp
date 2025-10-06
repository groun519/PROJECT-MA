// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Movement/GAM_Rush.h"

#include "AbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "Player/MAPlayerCharacter.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"


UGAM_Rush::UGAM_Rush()
{
}

void UGAM_Rush::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (PlayerCharacter)
	{
		GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(PlayerCharacter->RushingTag);
	}
	
	
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MontageToPlay);
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UGAM_Rush::K2_EndAbility);
		//MontageTask->OnBlendOut.AddDynamic(this, &UGAM_Rush::K2_EndAbility);
		MontageTask->OnInterrupted.AddDynamic(this, &UGAM_Rush::K2_EndAbility);
		MontageTask->OnCancelled.AddDynamic(this, &UGAM_Rush::K2_EndAbility);
		MontageTask->ReadyForActivation();
	}
	

	WaitInputRelease();
}

void UGAM_Rush::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (PlayerCharacter)
	{
		GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(PlayerCharacter->RushingTag);
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}



void UGAM_Rush::WaitInputRelease()
{
	UAbilityTask_WaitInputRelease* WaitInputTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	WaitInputTask->OnRelease.AddDynamic(this, &UGAM_Rush::HandleInputReleased);
	WaitInputTask->ReadyForActivation();
}

void UGAM_Rush::HandleInputReleased(float TimeWaited)
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character && Character->GetMesh() && Character->GetMesh()-> GetAnimInstance())
	{
		Character->GetMesh()-> GetAnimInstance()->Montage_JumpToSection(TEXT("End"),MontageToPlay);
	}
}
