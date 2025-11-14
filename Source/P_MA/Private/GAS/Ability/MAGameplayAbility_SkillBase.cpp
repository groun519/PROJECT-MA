// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/MAGameplayAbility_SkillBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "Player/MAPlayerCharacter.h"

UMAGameplayAbility_SkillBase::UMAGameplayAbility_SkillBase()
{
	BlockAbilitiesWithTag.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActiveBehaviorTag = FGameplayTag();
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

	bCooldownApplied = false;
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
	
	const FGameplayTagContainer& DynamicTags = GetCurrentAbilitySpec()->DynamicAbilityTags;
	
	FGameplayTagContainer AttributeFilter = DynamicTags.Filter(FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Ability.Attribute")));
	//동적 스킬 속성 태그 
	if (AttributeFilter.Num() > 0)
	{
		SkillElementTag = AttributeFilter.First();
	}
	
	ActiveBehaviorTag = FGameplayTag();
	FGameplayTagContainer BehaviorFilter = DynamicTags.Filter(FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Ability.Behavior")));
	//동적 스킬 행동 태그
	if (BehaviorFilter.Num() > 0)
	{
		ActiveBehaviorTag = BehaviorFilter.First();
	}
	else
	{
		ActiveBehaviorTag = SkillBehaviorTag;
	}

	if (ActiveBehaviorTag.IsValid())
		ActiveSkillBehavior = BehaviorModules.FindRef(ActiveBehaviorTag);

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
			if (ActiveSkillBehavior->IsApplyCooldownImmediate())
			{
				ApplyDefaultCooldownOnce();
			}
		}
		ActiveSkillBehavior->OwningAbility = this;
		ActiveSkillBehavior->OnActivate();

		UAnimMontage* MontageToPlay = ActiveSkillBehavior->MontageToPlay;
		if (MontageToPlay)
		{
			UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,MontageToPlay);
			PlayMontageTask->OnBlendOut.AddDynamic(this, &UMAGameplayAbility_SkillBase::K2_EndAbility);
			PlayMontageTask->OnCompleted.AddDynamic(this, &UMAGameplayAbility_SkillBase::K2_EndAbility);
			PlayMontageTask->OnCancelled.AddDynamic(this, &UMAGameplayAbility_SkillBase::ApplyShortCooldownAndRequestEndAbility);
			PlayMontageTask->OnInterrupted.AddDynamic(this, &UMAGameplayAbility_SkillBase::ApplyShortCooldownAndRequestEndAbility);
			PlayMontageTask->ReadyForActivation();
		}
	}
}

void UMAGameplayAbility_SkillBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!bWasCancelled && !bCooldownApplied)
	{	//취소되지 않고 쿨타임 적용 안됐다면 풀쿨타임
		bCooldownApplied = true;
		float CooldownToApply = 10.f;
		if (ActiveSkillBehavior)
		{
			CooldownToApply = ActiveSkillBehavior->CooldownDuration;
		}
		ApplyBehaviorCooldown(CooldownToApply);
	}
	else if (bWasCancelled && !bCooldownApplied)
	{	//취소된 스킬이라면 짧은 쿨타임
		bCooldownApplied = true;
		float CooldownToApply = 1.f;
		if (ActiveSkillBehavior)
		{
			CooldownToApply = ActiveSkillBehavior->ShortCoolDownDuration;
		}
		ApplyBehaviorCooldown(CooldownToApply);
	}
	
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

/***********************************************************************************/
/*										Damage									   */
/***********************************************************************************/
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
	if (!DamageEffect || !HasAuthority(&CurrentActivationInfo))
		return;

	TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetActorsFromTargetData(TargetData, 0);
	for (AActor* TargetActor : TargetActors)
	{
		if (TargetActor && !IgnoreTargets.Contains(TargetActor))
		{
			FGameplayAbilityTargetDataHandle SingleTargetHandle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(TargetActor);
			BP_ApplyGameplayEffectToTarget(SingleTargetHandle, DamageEffect, GetAbilityLevel());
			IgnoreTargets.Add(TargetActor);
		}
	}
}

/***********************************************************************************/
/*										Cooldown								   */
/***********************************************************************************/
const FGameplayTagContainer* UMAGameplayAbility_SkillBase::GetCooldownTags() const
{
	if (SharedCooldownTag.IsValid())
	{
		static FGameplayTagContainer TagContainer;
		TagContainer.Reset();
		TagContainer.AddTag(SharedCooldownTag);
		return &TagContainer;
	}
	return Super::GetCooldownTags();
}
UGameplayEffect* UMAGameplayAbility_SkillBase::GetCooldownGameplayEffect() const
{
	return nullptr;
}

void UMAGameplayAbility_SkillBase::ApplyDefaultCooldownOnce()
{
	if (bCooldownApplied)
		return;
	if (!HasAuthority(&CurrentActivationInfo))
		return;
	bCooldownApplied = true;
	float CooldownToApply = 1.0f;
	if (ActiveSkillBehavior)
		CooldownToApply = ActiveSkillBehavior->CooldownDuration;
	ApplyBehaviorCooldown(CooldownToApply);
}

void UMAGameplayAbility_SkillBase::ApplyShortCooldownAndRequestEndAbility()
{
	if (bCooldownApplied)
	{
		EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true);
		return;
	}
	bCooldownApplied = true;
	
	float CooldownToApply = 0.1f;
	if (ActiveSkillBehavior)
		CooldownToApply = ActiveSkillBehavior->ShortCoolDownDuration;

	ApplyBehaviorCooldown(CooldownToApply);
	
	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true);
}

void UMAGameplayAbility_SkillBase::ApplyBehaviorCooldown(float CooldownToApply)
{
	if (!CooldownGE || !HasAuthority(&CurrentActivationInfo))
		return;

	const float FinalDuration =  CooldownToApply;
	if (FinalDuration <= 0)
		return;
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE, GetAbilityLevel());
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(CooldownDurationTag, FinalDuration);
		ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);
	}
}
