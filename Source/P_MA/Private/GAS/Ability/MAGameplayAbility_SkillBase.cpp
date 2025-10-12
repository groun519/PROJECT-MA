// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "AbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/WeaponEffectInterface.h"

UMAGameplayAbility_SkillBase::UMAGameplayAbility_SkillBase()
{
	DamageEventTag = UMAAbilitySystemStatics::GetMontageDamageTag();
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
	bool bIsChargingSkill = ModuleBehaviorTag.MatchesTag(UMAAbilitySystemStatics::GetChargeSkillTag());
	bool bIsChainSkill = ModuleBehaviorTag.MatchesTag(UMAAbilitySystemStatics::GetChainSkillTag());
	bool bIsHoldingSkill = ModuleBehaviorTag.MatchesTag(UMAAbilitySystemStatics::GetHoldSkillTag());

	if (bIsChargingSkill)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skill charges"));
	}
	else if (bIsChainSkill)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skill chains"));
	}
	else if (bIsHoldingSkill)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skill holds"));
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Default Skill"));
	
	K2_EndAbility();
}
