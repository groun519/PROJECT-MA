// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Movement/MovementBehavior_Rush.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "Player/MAPlayerCharacter.h"

void UMovementBehavior_Rush::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();

	bIsEnd = false;

	OwningAbility->GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(PlayerCharacter->RushingTag);
	
	WaitInputRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(OwningAbility);
	WaitInputRelease->OnRelease.AddDynamic(this, &UMovementBehavior_Rush::OnInputReleased);
	WaitInputRelease->ReadyForActivation();

	TimeoutTask = UAbilityTask_WaitDelay::WaitDelay(OwningAbility, MaxRushDuration);
	TimeoutTask->OnFinish.AddDynamic(this, &UMovementBehavior_Rush::OnFinished);
	TimeoutTask->ReadyForActivation();
	
	WaitClearEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, IgnoreClearTag);
	WaitClearEventTask->EventReceived.AddDynamic(this, &UMovementBehavior_Rush::ClearIgnore);
	WaitClearEventTask->ReadyForActivation();
	
	if (OwningAbility->K2_HasAuthority())
	{
		WaitDamageTagEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility,DamageEventTag);
		WaitDamageTagEventTask->EventReceived.AddDynamic(this, &UMovementBehavior_Rush::OnDamageEventReceived);
		WaitDamageTagEventTask->ReadyForActivation();
	}
}

void UMovementBehavior_Rush::OnEndAbility_Implementation()
{
	if (WaitInputRelease.IsValid())
		WaitInputRelease->EndTask();
	if (TimeoutTask.IsValid())
		TimeoutTask->EndTask();
	if (WaitDamageTagEventTask.IsValid())
		WaitDamageTagEventTask->EndTask();
	if (WaitClearEventTask.IsValid())
		WaitClearEventTask->EndTask();
	
	Super::OnEndAbility_Implementation();
}

void UMovementBehavior_Rush::OnInputReleased(float TimeHeld)
{
	if (bIsEnd)
		return;
	bIsEnd = true;
	
	if (TimeHeld <= 0.2f)
	{
		ApplyCooldownAndEndAbility(ShortCooldownEffect);
		OwningAbility->GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(PlayerCharacter->RushingTag);
		return;
	}
	if (OwningAbility)
	{
		OwningAbility->MontageToOtherSection(FName("End"));
		if (CooldownGE)
		{
			ApplyCooldownAndEndAbility(CooldownGE);
		}
	}
	OwningAbility->GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(PlayerCharacter->RushingTag);
}

void UMovementBehavior_Rush::OnFinished()
{
	if (bIsEnd)
		return;
	bIsEnd = true;
	if (OwningAbility)
	{
		OwningAbility->MontageToOtherSection(FName("End"));
		if (CooldownGE)
		{
			OwningAbility->ApplyEffectToOwner(CooldownGE);
		}
	}
	OwningAbility->GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(PlayerCharacter->RushingTag);
}

void UMovementBehavior_Rush::OnDamageEventReceived(FGameplayEventData Payload)
{
	if (OwningAbility->K2_HasAuthority())
	{
		TArray<FHitResult> HitResults = OwningAbility->GetHitResultFromVirtualSocketTargetData(Payload.TargetData);
		OwningAbility->ApplyDamageToHitResults(HitResults, DamageEffect);
	}
}

void UMovementBehavior_Rush::ClearIgnore(FGameplayEventData Payload)
{
	if (OwningAbility->K2_HasAuthority())
	{
		OwningAbility->IgnoreTargets.Empty();
	}
}
