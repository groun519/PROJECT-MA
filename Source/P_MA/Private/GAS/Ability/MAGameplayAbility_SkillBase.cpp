// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "Player/MAPlayerCharacter.h"

UMAGameplayAbility_SkillBase::UMAGameplayAbility_SkillBase()
{
	BlockAbilitiesWithTag.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UMAGameplayAbility_SkillBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	/*
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
*/
	IgnoreTargets.Empty();
	/*
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
	*/
	
	
	// Module 3) Behavior	-	동적으로 변한 태그 확인
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

	UAnimMontage* MontageToPlay = ActiveSkillBehavior->MontageToPlay;
	if (MontageToPlay)
	{
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,MontageToPlay);
		PlayMontageTask->OnBlendOut.AddDynamic(this, &UMAGameplayAbility_SkillBase::K2_EndAbility);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UMAGameplayAbility_SkillBase::K2_EndAbility);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UMAGameplayAbility_SkillBase::K2_EndAbility);
		PlayMontageTask->OnCompleted.AddDynamic(this, &UMAGameplayAbility_SkillBase::K2_EndAbility);
		PlayMontageTask->ReadyForActivation();
	}
	
	if (ActiveSkillBehavior)
	{
		AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(ActorInfo->AvatarActor.Get());
		if (PlayerCharacter)
		{
			if (ActiveSkillBehavior->ShouldLockRotation())
			{
				GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(UMAAbilitySystemStatics::GetRotationLockTag());
			}
			if (!ActiveSkillBehavior->IsRequirePlayerInput())
			{
				PlayerCharacter->SetInputEnabledFromPlayerController(false);
			}
		}
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
		AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(ActorInfo->AvatarActor.Get());
		if (PlayerCharacter)
		{
			if (ActiveSkillBehavior->ShouldLockRotation())
			{
				GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(UMAAbilitySystemStatics::GetRotationLockTag());
			}
			if (!ActiveSkillBehavior->IsRequirePlayerInput())
			{
				PlayerCharacter->SetInputEnabledFromPlayerController(true);
			}
		}
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
			AnimInst->Montage_JumpToSection(SectionName,ActiveSkillBehavior->MontageToPlay);
		}
	}
}

void UMAGameplayAbility_SkillBase::RequestEndAbility()
{
	EndAbility(GetCurrentAbilitySpecHandle(),GetCurrentActorInfo(),GetCurrentActivationInfo(),true,false);
}


void UMAGameplayAbility_SkillBase::ApplyDamageToHitResults(const TArray<FHitResult>& HitResults,
	TSubclassOf<UGameplayEffect> DamageEffect)
{
	if (!DamageEffect || !HasAuthority(&CurrentActivationInfo))
		return;

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && !IgnoreTargets.Contains(HitActor))
		{
			ApplyGameplayEffectToHitResultActor(Hit, DamageEffect, GetAbilityLevel());
			IgnoreTargets.Add(HitActor);
		}
	}
}

void UMAGameplayAbility_SkillBase::ApplyDamageToTargetData(const FGameplayAbilityTargetDataHandle& TargetData,
	TSubclassOf<UGameplayEffect> DamageEffect)
{
	if (!DamageEffect)
		return;

	BP_ApplyGameplayEffectToTarget(TargetData, DamageEffect, GetAbilityLevel());
}

void UMAGameplayAbility_SkillBase::ApplyEffectToOwner(TSubclassOf<UGameplayEffect> Effect, float Level)
{
	if (!Effect)
		return;
	BP_ApplyGameplayEffectToOwner(Effect,Level);
}
