// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/GameplayAbility_Dash.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/MACharacter.h"
#include "GameFramework/Character.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Passive/GAP_Movement.h"

UGameplayAbility_Dash::UGameplayAbility_Dash()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	BlockAbilitiesWithTag.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGameplayAbility_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
	
	UAbilityTask_PlayMontageAndWait* PlayMontageTask =UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None, SkillMontage);
	PlayMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_Dash::K2_EndAbility);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_Dash::K2_EndAbility);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_Dash::K2_EndAbility);
	PlayMontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitStartDashTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,FGameplayTag::RequestGameplayTag("Ability.Movement.Dash.Start"));
	WaitStartDashTask->EventReceived.AddDynamic(this, &UGameplayAbility_Dash::Server_ExecuteDash);
	WaitStartDashTask->ReadyForActivation();
}



void UGameplayAbility_Dash::Server_ExecuteDash_Implementation(FGameplayEventData EventData)
{
	if (K2_HasAuthority())
	{
		
		const FVector ForwardVector = GetAvatarActorFromActorInfo()->GetActorForwardVector();
		const FVector DashVelocity = ForwardVector * DashSpeed;
		
		UE_LOG(LogTemp, Warning, TEXT("[Server_executedash] %s"), *ForwardVector.ToString());
		UE_LOG(LogTemp, Warning, TEXT("[Server_executedash] %s"), *DashVelocity.ToString());
		FGameplayTag PushActionTag = FGameplayTag::RequestGameplayTag("Ability.Passive.Dash.Activate");
		PushTarget(GetAvatarActorFromActorInfo(), DashVelocity, PushActionTag);
	}
}
