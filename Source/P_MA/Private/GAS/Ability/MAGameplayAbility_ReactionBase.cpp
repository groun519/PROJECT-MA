// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/MAGameplayAbility_ReactionBase.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/MACharacter.h"
#include "GAS/MAAbilitySystemStatics.h"

UMAGameplayAbility_ReactionBase::UMAGameplayAbility_ReactionBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Ability.Attack"));
	ActivationOwnedTags.AddTag(UMAAbilitySystemStatics::GetAnyReactionStateTag());

	CancelTagsOnHit.AddTag(FGameplayTag::RequestGameplayTag("Ability.Attack"));
}

void UMAGameplayAbility_ReactionBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{	
	AMACharacter* AvatarChar = Cast<AMACharacter>(GetAvatarActorFromActorInfo());
	if (!AvatarChar || !TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FGameplayTag ReactionTag = TriggerEventData->EventTag;
	if (const FGameplayTag* ImmuneTag = ReactionToImmunityTagMap.Find(ReactionTag))
	{
		if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(*ImmuneTag))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CancelTagsOnHit.IsValid())
	{
		ActorInfo->AbilitySystemComponent->CancelAbilities(&CancelTagsOnHit);
	}
	
	FReactionAnimConfig AnimConfig;
	if (!AvatarChar->GetReactionAnimConfig(ReactionTag, AnimConfig) || !AnimConfig.Montage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	float HitForce = TriggerEventData->EventMagnitude;
	FVector PushDir = GetPushDirection(AvatarChar, Cast<AActor>(TriggerEventData->Instigator));
	PushDir.Z = AnimConfig.VerticalLaunchScale;
	AvatarChar->LaunchCharacter(PushDir * HitForce, true, true);
	
	if (const FGameplayTag* FoundDebuff = ReactionToDebuffTagMap.Find(ReactionTag))
	{
		CurrentDebuffTag = *FoundDebuff;
		ActorInfo->AbilitySystemComponent->AddLooseGameplayTag(CurrentDebuffTag);
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AnimConfig.Montage);
	MontageTask->OnCompleted.AddDynamic(this, &UMAGameplayAbility_ReactionBase::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UMAGameplayAbility_ReactionBase::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UMAGameplayAbility_ReactionBase::OnMontageCompleted);
	MontageTask->ReadyForActivation();
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UMAGameplayAbility_ReactionBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (CurrentDebuffTag.IsValid() && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveLooseGameplayTag(CurrentDebuffTag);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMAGameplayAbility_ReactionBase::OnMontageCompleted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

FVector UMAGameplayAbility_ReactionBase::GetPushDirection(const AActor* Avatar, const AActor* Attacker) const
{
	if (!Avatar)
		return FVector::ZeroVector;
	if (!Attacker)
		return -Avatar->GetActorForwardVector();

	FVector Dir = (Avatar->GetActorLocation() - Attacker->GetActorLocation()).GetSafeNormal();
	Dir.Z = 0.f;
	return Dir;
}

