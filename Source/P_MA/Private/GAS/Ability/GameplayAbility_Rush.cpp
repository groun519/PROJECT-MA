// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/GameplayAbility_Rush.h"

#include "AbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UGameplayAbility_Rush::UGameplayAbility_Rush()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	BlockAbilitiesWithTag.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGameplayAbility_Rush::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                            const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("--- STEP 0: ActivateAbility Called. Applying effect and waiting for End Event... ---"));

	ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, RushingEffectClass.GetDefaultObject(), GetAbilityLevel());

	UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
	if (AnimInstance && SkillMontage)
	{
		AnimInstance->Montage_Play(SkillMontage);
		AnimInstance->Montage_JumpToSection(TEXT("Start"), SkillMontage);
	}

	FGameplayTag EndRushTag = FGameplayTag::RequestGameplayTag("Ability.Movement.Rush.End");
	UAbilityTask_WaitGameplayEvent* WaitEndEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EndRushTag);
	WaitEndEventTask->EventReceived.AddDynamic(this, &UGameplayAbility_Rush::HandleEndEvent);
	WaitEndEventTask->ReadyForActivation();
}

void UGameplayAbility_Rush::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	UE_LOG(LogTemp, Warning, TEXT("--- STEP 2/7: [CLIENT] InputReleased Called. Calling Server_EndRush RPC... ---"));
	Server_EndRush();
}

void UGameplayAbility_Rush::Server_EndRush_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("--- STEP 3/7: [SERVER] Server_EndRush RPC Received. Calling Multicast_EndRush RPC... ---"));
	Multicast_EndRush();
}

void UGameplayAbility_Rush::Multicast_EndRush_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("--- STEP 4/7: [ALL] Multicast_EndRush RPC Received. Jumping montage to 'End' section... ---"));
	UAnimInstance* AnimInstance = CurrentActorInfo->GetAnimInstance();
	if (AnimInstance && SkillMontage)
	{
		AnimInstance->Montage_JumpToSection(TEXT("End"), SkillMontage);
	}
}

void UGameplayAbility_Rush::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                       const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                       bool bReplicateEndAbility, bool bWasCancelled)
{
	UE_LOG(LogTemp, Warning, TEXT("--- STEP 6/7: EndAbility Called. Removing Rushing effect... ---"));

	if (ActorInfo->AbilitySystemComponent.IsValid())
	{
		FGameplayEffectQuery Query;
		Query.EffectDefinition = RushingEffectClass;
		int32 RemovedCount = ActorInfo->AbilitySystemComponent->RemoveActiveEffects(Query);
		UE_LOG(LogTemp, Warning, TEXT("--- STEP 7/7: Removed %d rushing effects. ---"), RemovedCount);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_Rush::HandleEndEvent(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("--- STEP 5/7: HandleEndEvent Called by AnimNotify. Calling K2_EndAbility... ---"));
	K2_EndAbility();
}
