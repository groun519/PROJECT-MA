// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/SkillBehavior_Chain.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/MAGameplayAbility.h"
#include "GameplayTagsManager.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"


void USkillBehavior_Chain::OnActivate_Implementation()
{
	if (!OwningAbility)
		return;
	Super::OnActivate_Implementation();
	
	OwningAbility->IgnoreTargets.Empty();
	bIsComboInputBuffered = false;

	WaitComboChangeEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility,ComboChangeEventTag,nullptr,false,false);
	WaitComboChangeEventTask->EventReceived.AddDynamic(this, &USkillBehavior_Chain::ComboChangedEventReceived);
	WaitComboChangeEventTask->ReadyForActivation();
	
	WaitClearEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, ComboClearEventTag);
	WaitClearEventTask->EventReceived.AddDynamic(this, &USkillBehavior_Chain::ClearIgnore);
	WaitClearEventTask->ReadyForActivation();

	if (OwningAbility->K2_HasAuthority())
	{
		WaitHitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, DamageEventTag);
		WaitHitEventTask->EventReceived.AddDynamic(this, &USkillBehavior_Chain::HitTarget);
		WaitHitEventTask->ReadyForActivation();
	}
	SetupWaitComboInputPress();
}

void USkillBehavior_Chain::OnEndAbility_Implementation()
{
	if (CooldownGE)
		OwningAbility->ApplyEffectToOwner(CooldownGE);
	
	if (WaitComboChangeEventTask.IsValid())
		WaitComboChangeEventTask->EndTask();
	if (WaitHitEventTask.IsValid())
		WaitHitEventTask->EndTask();
	if (WaitClearEventTask.IsValid())
		WaitClearEventTask->EndTask();
	if (WaitInputPress.IsValid())
		WaitInputPress->EndTask();
	
	Super::OnEndAbility_Implementation();
}

void USkillBehavior_Chain::SetupWaitComboInputPress()
{
	WaitInputPress = UAbilityTask_WaitInputPress::WaitInputPress(OwningAbility);
	WaitInputPress->OnPress.AddDynamic(this, &USkillBehavior_Chain::HandleInputPress);
	WaitInputPress->ReadyForActivation();
}

void USkillBehavior_Chain::ComboChangedEventReceived(FGameplayEventData EventData)
{
	FGameplayTag EventTag = EventData.EventTag;
	if (EventTag == ComboEndEventTag)
		return;
	
	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag,TagNames);
	NextComboName = TagNames.Last();

	bIsComboInputBuffered = false;
}

void USkillBehavior_Chain::HitTarget(FGameplayEventData EventData)
{
	TArray<FHitResult> HitResults = OwningAbility->GetHitResultFromVirtualSocketTargetData(EventData.TargetData);

	for (const FHitResult& HitResult : HitResults)
	{
		if (OwningAbility->IgnoreTargets.Contains(HitResult.GetActor())) continue;
			
		TSubclassOf<UGameplayEffect> GameplayEffect = GetDamageEffectForCurrentCombo();
		OwningAbility->ApplyGameplayEffectToHitResultActor(HitResult, GameplayEffect, OwningAbility->GetAbilityLevel());
		OwningAbility->IgnoreTargets.Add(HitResult.GetActor());
	}
}

void USkillBehavior_Chain::ClearIgnore(FGameplayEventData EventData)
{
	if (OwningAbility->K2_HasAuthority())
	{
		OwningAbility->IgnoreTargets.Empty();
	}

	if (bIsComboInputBuffered && NextComboName != NAME_None)
	{
		UAnimInstance* OwerAnimInst = OwningAbility->GetOwnerAnimInstance();
		if (OwerAnimInst)
		{
			OwerAnimInst->Montage_JumpToSection(NextComboName, MontageToPlay);
		}
	}
	bIsComboInputBuffered = false;
	NextComboName = NAME_None;
}

void USkillBehavior_Chain::HandleInputPress(float Time)
{
	bIsComboInputBuffered = true;
	SetupWaitComboInputPress();
}

TSubclassOf<UGameplayEffect> USkillBehavior_Chain::GetDamageEffectForCurrentCombo() const
{
	UAnimInstance* OwnerAnimInstance = OwningAbility->GetOwnerAnimInstance();
	if (OwnerAnimInstance)
	{
		FName CurrentSectionName = OwnerAnimInstance->Montage_GetCurrentSection(MontageToPlay);
		const TSubclassOf<UGameplayEffect>* FoundEffectPtr = DamageEffectMap.Find(CurrentSectionName);
		if (FoundEffectPtr)
			return *FoundEffectPtr;
	}
	return DamageEffect;
}

void USkillBehavior_Chain::TryCommitCombo()
{
	if (NextComboName == NAME_None)
		return;

	UAnimInstance* OwnerAnimInst = OwningAbility->GetOwnerAnimInstance();
	if (!OwnerAnimInst)
		return;

	OwnerAnimInst->Montage_SetNextSection(OwnerAnimInst->Montage_GetCurrentSection(MontageToPlay), NextComboName, MontageToPlay);
}

