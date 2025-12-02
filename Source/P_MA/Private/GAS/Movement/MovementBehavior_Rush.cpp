// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Movement/MovementBehavior_Rush.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "GAS/Ability/SkillBehaviorConfig.h"
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
}

void UMovementBehavior_Rush::OnEndAbility_Implementation()
{
	if (WaitInputRelease.IsValid())
		WaitInputRelease->EndTask();
	if (TimeoutTask.IsValid())
		TimeoutTask->EndTask();

	if (WaitClearEventTask.IsValid())
		WaitClearEventTask->EndTask();
	
	Super::OnEndAbility_Implementation();
}

void UMovementBehavior_Rush::InitFromConfig(const FInstancedStruct& ConfigPayload)
{
	Super::InitFromConfig(ConfigPayload);
	const FConfig_Rush* RushConfig = ConfigPayload.GetPtr<FConfig_Rush>();
	if (RushConfig)
	{
		MontageToPlay=RushConfig->MontageToPlay;
		MaxRushDuration = RushConfig->MaxRushDuration;
		VFXDataSet=RushConfig->VFXDataSet;
	}
}


void UMovementBehavior_Rush::OnInputReleased(float TimeHeld)
{
	if (bIsEnd)
		return;
	bIsEnd = true;
	
	if (TimeHeld <= 0.2f)
	{
		OwningAbility->ApplyShortCooldownAndRequestEndAbility();
		OwningAbility->GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(PlayerCharacter->RushingTag);
		return;
	}
	OwningAbility->ApplyDefaultCooldownOnce();
	MontageToOtherSection(FName("End"));
	
	OwningAbility->GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(PlayerCharacter->RushingTag);
}

void UMovementBehavior_Rush::OnFinished()
{
	if (bIsEnd)
		return;
	bIsEnd = true;
	MontageToOtherSection(FName("End"));
	OwningAbility->ApplyDefaultCooldownOnce();
	
	OwningAbility->GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(PlayerCharacter->RushingTag);
}

void UMovementBehavior_Rush::ClearIgnore(FGameplayEventData Payload)
{
	if (OwningAbility->K2_HasAuthority())
	{
		OwningAbility->IgnoreTargets.Empty();
	}
}
