// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Modules/SkillModule_Instant.h"

#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "GAS/Ability/MAGameplayAbility_Skill.h"

void USkillModule_Instant::OnAbilityActivated()
{
	if (!OwnerSkill)	return;

	const FSkillData& SkillData = OwnerSkill->GetSkillData();
	if (!SkillData.SkillMontage)
	{
		OwnerSkill->EndAbility(OwnerSkill->GetCurrentAbilitySpecHandle(), OwnerSkill->GetCurrentActorInfo(), OwnerSkill->GetCurrentActivationInfo(), true, false);
		return;
	}
	
	StartMontageTask();
	if (SkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Melee")))
	{
		StartWaitDamageEventTask(FName("Event.Montage.Damage"));
	}
	if (SkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Projectile")))
	{
		StartWaitDamageEventTask(FName("Event.Montage.SpawnProjectile"));
	}
}

void USkillModule_Instant::OnAbilityEnded(bool bWasCancelled)
{
	if (MontageTask)	MontageTask->EndTask();
	if (DamageEventTask)		DamageEventTask->EndTask();
}

void USkillModule_Instant::StartMontageTask()
{
	if (!OwnerSkill)	return;
	const FSkillData& SkillData = OwnerSkill->GetSkillData();

	float PlayRate = OwnerSkill->GetTotalAnimSpeed();

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(OwnerSkill,NAME_None,SkillData.SkillMontage,PlayRate,NAME_None,false);
	MontageTask->OnCompleted.AddDynamic(this, &USkillModule_Instant::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &USkillModule_Instant::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &USkillModule_Instant::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void USkillModule_Instant::OnMontageEnded()
{
	if (OwnerSkill)
	{
		OwnerSkill->EndAbility(OwnerSkill->GetCurrentAbilitySpecHandle(), OwnerSkill->GetCurrentActorInfo(), OwnerSkill->GetCurrentActivationInfo(), true, false);
	}
}

void USkillModule_Instant::StartWaitDamageEventTask(FName TagName)
{
	if (!OwnerSkill)	return;
	FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(TagName);

	DamageEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwnerSkill,EventTag,nullptr,false,true);
	DamageEventTask->EventReceived.AddDynamic(this, &USkillModule_Instant::OnDamageEventReceived);
	DamageEventTask->ReadyForActivation();
}

void USkillModule_Instant::OnDamageEventReceived(FGameplayEventData Payload)
{
	if (OwnerSkill)
	{
		OwnerSkill->ExecuteSkillAction(Payload, 1.f);
	}
}

