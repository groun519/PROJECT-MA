// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/MAGameplayAbility_ReactionBase.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/MACharacter.h"

UMAGameplayAbility_ReactionBase::UMAGameplayAbility_ReactionBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Ability.Attack"));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Ability.Attack"));
}

void UMAGameplayAbility_ReactionBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true,false);
		return;
	}
	AMACharacter* AvatarChar = Cast<AMACharacter>(GetAvatarActorFromActorInfo());
	if (!AvatarChar || !TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	FGameplayTag ReactionTag = TriggerEventData->EventTag;
	float Force = TriggerEventData->EventMagnitude;
	const AActor* Attacker = Cast<AActor>(TriggerEventData->Instigator);

	FName SectionName = FName("Front");
	FVector PushDirection = -AvatarChar->GetActorForwardVector();

	if (Attacker)
	{
		FVector DirToAttacker = (Attacker->GetActorLocation() - AvatarChar->GetActorLocation()).GetSafeNormal();
		DirToAttacker.Z = 0.f;
		PushDirection = -DirToAttacker;

		float ForwardDot = FVector::DotProduct(DirToAttacker, AvatarChar->GetActorForwardVector());
		float RightDot = FVector::DotProduct(DirToAttacker, AvatarChar->GetActorRightVector());

		if (ForwardDot >= 0.5f)			SectionName = FName("Front");
		else if (ForwardDot <= -0.5f)	SectionName = FName("Back");
		else if (RightDot >= 0.5f)		SectionName = FName("Right");
		else if (RightDot <= -0.5f)		SectionName = FName("Left");
	}

	UAnimMontage* MontageToPlay = AvatarChar->GetFlinchMontage();
	bool bIsPushReaction = ReactionTag.MatchesTag(FGameplayTag::RequestGameplayTag("Effect.Reaction.Knockback")) ||
			ReactionTag.MatchesTag(FGameplayTag::RequestGameplayTag("Effect.Reaction.Knockdown")) ||
			ReactionTag.MatchesTag(FGameplayTag::RequestGameplayTag("Effect.Reaction.Airborne"));

	if (bIsPushReaction)
	{
		if (AvatarChar->GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("State.Immune.Push")))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}
		
		if (ReactionTag.MatchesTag(FGameplayTag::RequestGameplayTag("Effect.Reaction.Knockback")))
		{
			PushDirection.Z = 0.2f;
			AvatarChar->LaunchCharacter(PushDirection * Force, true, true);
		}
		else if (ReactionTag.MatchesTag(FGameplayTag::RequestGameplayTag("Effect.Reaction.Airborne")))
		{
			PushDirection = FVector(0.f, 0.f, 1.f);
			AvatarChar->LaunchCharacter(PushDirection * Force, true, true);
		}
		else if (ReactionTag.MatchesTag(FGameplayTag::RequestGameplayTag("Effect.Reaction.Knockdown")))
		{
			PushDirection.Z = 0.5f;
			AvatarChar->LaunchCharacter(PushDirection * Force, true, true);
			MontageToPlay = AvatarChar->GetKnockdownMontage();
			SectionName = NAME_None; // 넉다운은 4방향 섹션이 없으므로 None 처리
		}
	}

	if (MontageToPlay)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, MontageToPlay, 1.f, SectionName);
		MontageTask->OnCompleted.AddDynamic(this, &UMAGameplayAbility_ReactionBase::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UMAGameplayAbility_ReactionBase::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UMAGameplayAbility_ReactionBase::OnMontageCompleted);
		MontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UMAGameplayAbility_ReactionBase::OnMontageCompleted()
{
	bool bWasCancelled = false;
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, bWasCancelled);
}

FVector UMAGameplayAbility_ReactionBase::GetPushDirectionFromEvent(const FGameplayEventData& EventData) const
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	const AActor* Attacker = Cast<AActor>(EventData.Instigator);

	if (Avatar && Attacker)
	{
		FVector Dir = (Avatar->GetActorLocation() - Attacker->GetActorLocation()).GetSafeNormal();
		Dir.Z = 0.f;
		return Dir;
	}
	return Avatar ? -Avatar->GetActorForwardVector() : FVector::ZeroVector;
}

FName UMAGameplayAbility_ReactionBase::GetFlinchSectionFromEvent(const FGameplayEventData& EventData) const
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	const AActor* Attacker = Cast<AActor>(EventData.Instigator);

	if (Avatar && Attacker)
	{
		FVector DirToAttacker = (Attacker->GetActorLocation() - Avatar->GetActorLocation()).GetSafeNormal();
		
		float ForwardDot = FVector::DotProduct(DirToAttacker, Avatar->GetActorForwardVector());
		float RightDot = FVector::DotProduct(DirToAttacker, Avatar->GetActorRightVector());

		if (ForwardDot >= 0.5f) return FName("Front");
		else if (ForwardDot <= -0.5f) return FName("Back");
		else if (RightDot >= 0.5f) return FName("Right");
		else return FName("Left");
	}
	
	// 공격자가 없으면 기본적으로 정면 피격
	return FName("Front");
}
