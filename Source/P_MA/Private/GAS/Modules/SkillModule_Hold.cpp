// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Modules/SkillModule_Hold.h"
#include "GAS/Ability/MAGameplayAbility_Skill.h"
#include "GAS/Modules/MASkillModuleData.h"

void USkillModule_Hold::OnAbilityActivated()
{
	if (!OwnerSkill)	return;

	const FSkillData& SkillData = OwnerSkill->GetSkillData();
	if (!SkillData.SkillMontage)
	{
		OwnerSkill->EndAbility(OwnerSkill->GetCurrentAbilitySpecHandle(), OwnerSkill->GetCurrentActorInfo(), OwnerSkill->GetCurrentActivationInfo(), true, false);
		return;
	}

	bIsHolding = true;
	CachedHoldMultiplier = 0.8f;
	CachedMaxHoldDuration = 2.5f;

	const FModuleBehaviorData& BehaviorData = OwnerSkill->GetBehaviorData();
	if (const FBehavior_Hold* Config = BehaviorData.ModuleConfig.GetPtr<FBehavior_Hold>())
	{
		CachedHoldMultiplier = Config->HoldingDamageMultiplier;
		CachedMaxHoldDuration = Config->MaxHoldDuration;
	}

	StartMontageTask();
	StartWaitJumpSectionEventTask();
	StartWaitInputReleaseTask();
	StartMaxHoldDelayTask();

	if (SkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Melee")))
	{
		StartWaitDamageEventTask(FName("Event.Montage.Damage"));
	}
	if (SkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Projectile")))
	{
		StartWaitDamageEventTask(FName("Event.Montage.SpawnProjectile"));
	}
}

void USkillModule_Hold::OnAbilityEnded(bool bWasCancelled)
{
	if (MontageTask)				MontageTask->EndTask();
	if (InputReleaseTask)			InputReleaseTask->EndTask();
	if (JumpMontageSectionTask)		JumpMontageSectionTask->EndTask();
	if (DamageEventTask)			DamageEventTask->EndTask();
	if (MaxHoldTask)				MaxHoldTask->EndTask();
}


void USkillModule_Hold::StartMontageTask()
{
	if (!OwnerSkill)	return;
	
	const FSkillData& SkillData = OwnerSkill->GetSkillData();
	float PlayRate = OwnerSkill->GetTotalAnimSpeed();

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(OwnerSkill, NAME_None, SkillData.SkillMontage,PlayRate,NAME_None,false);
	MontageTask->OnCompleted.AddDynamic(this, &USkillModule_Hold::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &USkillModule_Hold::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &USkillModule_Hold::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void USkillModule_Hold::OnMontageEnded()
{
	if (OwnerSkill)
	{
		OwnerSkill->EndAbility(OwnerSkill->GetCurrentAbilitySpecHandle(), OwnerSkill->GetCurrentActorInfo(), OwnerSkill->GetCurrentActivationInfo(), true, false);
	}
}

void USkillModule_Hold::StartWaitDamageEventTask(FName TagName)
{
	if (!OwnerSkill)	return;
	FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(TagName);

	DamageEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwnerSkill,EventTag,nullptr,false,true);
	DamageEventTask->EventReceived.AddDynamic(this, &USkillModule_Hold::OnDamageEventReceived);
	DamageEventTask->ReadyForActivation();
}

void USkillModule_Hold::OnDamageEventReceived(FGameplayEventData Payload)
{
	if (OwnerSkill)
	{
		OwnerSkill->ExecuteSkillAction(Payload, CachedHoldMultiplier);
	}
}

void USkillModule_Hold::StartWaitJumpSectionEventTask()
{
	if (!OwnerSkill)	return;
	FGameplayTag Tag = FGameplayTag::RequestGameplayTag("Event.Montage.JumpSection");

	JumpMontageSectionTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwnerSkill, Tag);
	JumpMontageSectionTask -> EventReceived.AddDynamic(this, &USkillModule_Hold::OnJumpSectionEventReceived);
	JumpMontageSectionTask -> ReadyForActivation();
}

void USkillModule_Hold::OnJumpSectionEventReceived(FGameplayEventData Payload)
{
	if (OwnerSkill)
	{
		OwnerSkill -> Montage_SetSection(FName("LoopStart"));
	}
}

void USkillModule_Hold::StartWaitInputReleaseTask()
{
	if (!OwnerSkill)	return;

	InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(OwnerSkill);
	InputReleaseTask -> OnRelease.AddDynamic(this, &USkillModule_Hold::OnInputRelease);
	InputReleaseTask -> ReadyForActivation();
}

void USkillModule_Hold::OnInputRelease(float TimeHeld)
{
	if (!bIsHolding)	return;

	if (OwnerSkill)
		OwnerSkill -> Montage_SetSection(FName("LoopEnd"));

	bIsHolding = false;
	if (MaxHoldTask)
			MaxHoldTask->EndTask();
}

void USkillModule_Hold::StartMaxHoldDelayTask()
{
	if (!OwnerSkill)	return;
	
	MaxHoldTask = UAbilityTask_WaitDelay::WaitDelay(OwnerSkill, CachedMaxHoldDuration);
	MaxHoldTask -> OnFinish.AddDynamic(this, &USkillModule_Hold::OnMaxHold);
	MaxHoldTask -> ReadyForActivation();
}

void USkillModule_Hold::OnMaxHold()
{
	if (!bIsHolding)	return;

	if (OwnerSkill)
		OwnerSkill -> Montage_SetSection(FName("LoopEnd"));

	bIsHolding = false;
	if (InputReleaseTask)
		InputReleaseTask->EndTask();
}
