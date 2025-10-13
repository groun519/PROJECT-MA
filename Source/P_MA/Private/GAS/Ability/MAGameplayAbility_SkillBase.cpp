// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/WeaponEffectInterface.h"
#include "Player/MAPlayerCharacter.h"

UMAGameplayAbility_SkillBase::UMAGameplayAbility_SkillBase()
{
	AttributeCueTag = UMAAbilitySystemStatics::GetSkillAttributeTag();
}

void UMAGameplayAbility_SkillBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,SkillAnimMontage);
	PlayMontageTask->OnBlendOut.AddDynamic(this, &UMAGameplayAbility_SkillBase::K2_EndAbility);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UMAGameplayAbility_SkillBase::K2_EndAbility);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &UMAGameplayAbility_SkillBase::K2_EndAbility);
	PlayMontageTask->OnCompleted.AddDynamic(this, &UMAGameplayAbility_SkillBase::K2_EndAbility);
	PlayMontageTask->ReadyForActivation();
	
	// --- Module 1) Utility ---
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		for (const TSubclassOf<UGameplayEffect>& UtilityEffect : ModuleUtility)
		{
			if (UtilityEffect)
			{
				FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(UtilityEffect);
				if (SpecHandle.IsValid())
					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}

	// Module 2) Attribute
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	IWeaponEffectInterface* WeaponEffect = Cast<IWeaponEffectInterface>(AvatarActor);
	if (WeaponEffect && AttributeEffects && ModuleAttributeTag.IsValid())
	{
		UNiagaraSystem* EffectToPlay = AttributeEffects->EffectMap.FindRef(ModuleAttributeTag);
		if (EffectToPlay)
			WeaponEffect->ActivateWeaponEffect(EffectToPlay);
	}

	/* 멀티 플레이어에서 변경되도록 Cue 사용은 작동 안함 - 내가 잘 모르나봄. K2_ExecuteGameplayCueWithParams() 써도 안되고 별 지랄..
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
	   if (ASC->GetOwnerRole() == ROLE_Authority)
	   {
		  FGameplayCueParameters CueParams;
		  CueParams.MatchedTagName = ModuleAttributeTag;
		  ASC->ExecuteGameplayCue(AttributeCueTag, CueParams);
	   }
	}
	*/

	// Module 3) Behavior
	FGameplayTag BehaviorTagToUse;
	const FGameplayTagContainer& DynamicTags = GetCurrentAbilitySpec()->DynamicAbilityTags;

	FGameplayTagContainer FilteredTags = DynamicTags.Filter(FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Ability.Behavior")));
	if (FilteredTags.Num() > 0)
	{
		BehaviorTagToUse = FilteredTags.First();
	}
	else
	{
		BehaviorTagToUse = DefaultBehaviorTag;
	}

	if (BehaviorTagToUse.IsValid())
		ActiveSkillBehavior = BehaviorModules.FindRef(BehaviorTagToUse);

	if (ActiveSkillBehavior)
	{
		ActiveSkillBehavior->OwningAbility = this;
		ActiveSkillBehavior->OnActivate();
	}
}

void UMAGameplayAbility_SkillBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActiveSkillBehavior)
	{
		ActiveSkillBehavior->OnEndAbility();
		ActiveSkillBehavior = nullptr;
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}



void UMAGameplayAbility_SkillBase::SetMontagePlayRate(float NewPlayRate)
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
		{
			UAnimMontage* ActiveMontage  = AnimInstance->GetCurrentActiveMontage();
			AnimInstance->Montage_SetPlayRate(ActiveMontage ,NewPlayRate);
		}
	}
}

void UMAGameplayAbility_SkillBase::MontageToOtherSection(FName SectionName)
{
	if (AMAPlayerCharacter* Character = Cast<AMAPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()))
	{
		if (UAnimInstance* AnimInst = Character->GetMesh()->GetAnimInstance())
		{
			AnimInst->Montage_JumpToSection(SectionName,SkillAnimMontage);
		}
	}
}
