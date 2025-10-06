// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Movement/MAGameplayAbility_Movement.h"

#include "AbilitySystemComponent.h"
#include "Player/MAPlayerCharacter.h"


UMAGameplayAbility_Movement::UMAGameplayAbility_Movement()
{
	
}

void UMAGameplayAbility_Movement::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (PlayerCharacter)
	{
		GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(RotationLock);
		PlayerCharacter->SetInputEnabledFromPlayerController(false);
	}
}

void UMAGameplayAbility_Movement::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (PlayerCharacter)
	{
		GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(RotationLock);
		PlayerCharacter->SetInputEnabledFromPlayerController(true);
	}
}


