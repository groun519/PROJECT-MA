// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/MAGameplayAbility_SkillBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MASkillVFXSet.h"
#include "GAS/PA_AbilitySystemGenerics.h"
#include "GAS/UtilityModule/UtilityModule.h"
#include "Player/MAPlayerCharacter.h"

UMAGameplayAbility_SkillBase::UMAGameplayAbility_SkillBase()
{
	BlockAbilitiesWithTag.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	CooldownDurationTag = FGameplayTag::RequestGameplayTag("Data.Cooldown.Duration");
	BehaviorModifierTag = UMAAbilitySystemStatics::GetBehaviorMultiplierTag();
	ElementalModifierTag = UMAAbilitySystemStatics::GetElementalMultiplierTag();
	VFXEventRootTag = FGameplayTag::RequestGameplayTag("Event.VFX");
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
	
	const FGameplayTagContainer& DynamicTags = GetCurrentAbilitySpec()->DynamicAbilityTags;
	UMAAbilitySystemComponent* ASC = Cast<UMAAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());

	ActiveBehaviorModule = nullptr;
	ActiveUtilityModule = nullptr;
	
	//속성 모듈 태그 결정
	ActiveSkillElementTag = DefaultElementTag;
	FGameplayTagContainer AttributeFilter = DynamicTags.Filter(FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Ability.Attribute")));
	if (AttributeFilter.Num() > 0)
	{
		ActiveSkillElementTag = AttributeFilter.First();
	}
	
	//유틸리티 모듈 태그 결정
	ActiveUtilityTag = DefaultUtilityTag;
	ActiveUtilityModule = nullptr;
	FGameplayTagContainer UtilityFilter = DynamicTags.Filter(FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Ability.Utility")));
	if (UtilityFilter.Num() > 0)
	{
		ActiveUtilityTag = UtilityFilter.First();
	}
	if (ASC && ASC->GetSystemGenerics())
	{
		ActiveUtilityModule = ASC->GetSystemGenerics()->FindSkillUtilityModuleByTag(ActiveUtilityTag);
	}
	
	//행동 모듈 태그 결정
	ActiveBehaviorTag = DefaultBehaviorTag;
	FGameplayTagContainer BehaviorFilter = DynamicTags.Filter(FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Ability.Behavior")));
	if (BehaviorFilter.Num() > 0)
	{
		ActiveBehaviorTag = BehaviorFilter.First();
	}
	ActiveBehaviorModule = BehaviorModules.FindRef(ActiveBehaviorTag);
	if (!ActiveBehaviorModule)
	{
		K2_EndAbility();
		return;
	}

	//모듈 실행
	if (ActiveBehaviorModule)
	{
		//즉시 쿨타임 적용
		if (ActiveBehaviorModule->IsApplyCooldownImmediate())
		{
			ApplyDefaultCooldownOnce();
		}
		//캐릭터 설정 (입력, 회전)
		AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(ActorInfo->AvatarActor.Get());
		if (PlayerCharacter)
		{
			if (ActiveBehaviorModule->ShouldLockRotation())
			{
				GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(UMAAbilitySystemStatics::GetRotationLockTag());
			}
			if (!ActiveBehaviorModule->IsRequirePlayerInput())
			{
				PlayerCharacter->SetInputEnabledFromPlayerController(false);
			}
		}

		if (ActiveUtilityModule)
		{	//스킬 시전 즉시 발동할 모듈
			ActiveUtilityModule->OwningAbility = this;
			ActiveUtilityModule->OnAbilityActivate();
		}
		
		ActiveBehaviorModule->OwningAbility = this;
		ActiveBehaviorModule->OnActivate();

		UAnimMontage* MontageToPlay = ActiveBehaviorModule->MontageToPlay;
		if (MontageToPlay)
		{
			float PlayRate = 1.f;
			if (ActiveUtilityModule)
			{
				PlayRate = ActiveUtilityModule->ModifyMontagePlayRate(PlayRate);
			}
			UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,MontageToPlay, PlayRate);
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
	if (ActiveUtilityModule)
	{	//스킬 종료 시 발동될 모듈
		ActiveUtilityModule->OnAbilityEnd(bWasCancelled);
	}
	
	if (!bWasCancelled && !bCooldownApplied)
	{	//취소되지 않고 쿨타임 적용 안됐다면 풀쿨타임
		bCooldownApplied = true;
		float CooldownToApply = 10.f;
		if (ActiveBehaviorModule)
		{
			CooldownToApply = ActiveBehaviorModule->CooldownDuration;
		}
		ApplyBehaviorCooldown(CooldownToApply);
	}
	else if (bWasCancelled && !bCooldownApplied)
	{	//취소된 스킬이라면 짧은 쿨타임
		bCooldownApplied = true;
		float CooldownToApply = 1.f;
		if (ActiveBehaviorModule)
		{
			CooldownToApply = ActiveBehaviorModule->ShortCoolDownDuration;
		}
		ApplyBehaviorCooldown(CooldownToApply);
	}
	
	if (ActiveBehaviorModule)
	{
		AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(ActorInfo->AvatarActor.Get());
		if (PlayerCharacter)
		{
			if (ActiveBehaviorModule->ShouldLockRotation())
			{
				GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(UMAAbilitySystemStatics::GetRotationLockTag());
			}
			if (!ActiveBehaviorModule->IsRequirePlayerInput())
			{
				PlayerCharacter->SetInputEnabledFromPlayerController(true);
			}
		}
		ActiveBehaviorModule->OnEndAbility();
		ActiveBehaviorModule = nullptr;
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMAGameplayAbility_SkillBase::ApplyGESpecToOwner(FGameplayEffectSpecHandle SpecHandle)
{
	if (SpecHandle.IsValid())
	{
		ApplyGameplayEffectSpecToOwner(GetCurrentAbilitySpecHandle(),GetCurrentActorInfo(),GetCurrentActivationInfo(),SpecHandle);
	}
}


UDataTable* UMAGameplayAbility_SkillBase::GetElementDataTable() const
{
	UMAAbilitySystemComponent* ASC = Cast<UMAAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (ASC && ASC->GetSystemGenerics())
	{
		return const_cast <UDataTable*>(ASC->GetSystemGenerics()->GetElementDataTable());
	}
	return nullptr;
}

/***********************************************************************************/
/*										Damage									   */
/***********************************************************************************/
const F_ElementInfoRow* UMAGameplayAbility_SkillBase::GetActiveElementInfoRow()
{
	const UDataTable* ElementDT = GetElementDataTable();
	if (ElementDT)
	{
		TArray<FName> TagNames;
		UGameplayTagsManager::Get().SplitGameplayTagFName(ActiveSkillElementTag,TagNames);
		FName LastName = TagNames.Last();
		return ElementDT->FindRow<F_ElementInfoRow>(LastName,"");
	}
	return nullptr;
}
void UMAGameplayAbility_SkillBase::ApplyDamageToHitResults(const TArray<FHitResult>& HitResults)
{
	if (!BaseDamageEffect || !HasAuthority(&CurrentActivationInfo))
		return;

	const F_ElementInfoRow* ElementInfoRow = GetActiveElementInfoRow();
	
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(BaseDamageEffect, GetAbilityLevel());
	if (!DamageSpecHandle.IsValid())
		return;
	//유틸리티 모듈의 데미지 배율
	if (ActiveUtilityModule)
	{
		ActiveUtilityModule->ModifyDamageEffectSpec(DamageSpecHandle);
	}
	//속성 모듈 데미지 배율
	if (ElementInfoRow && ElementInfoRow->ElementalDamageMultiplier != 1.f)
	{
		DamageSpecHandle.Data->SetSetByCallerMagnitude(ElementalModifierTag,ElementInfoRow->ElementalDamageMultiplier);
	}
	//행동 모듈 데미지 배율
	if (ActiveBehaviorModule)
	{
		DamageSpecHandle.Data->SetSetByCallerMagnitude(BehaviorModifierTag,ActiveBehaviorModule->GetCurrentDamageMultiplier());
	}
	
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && !IgnoreTargets.Contains(HitActor))
		{
			FGameplayEffectContextHandle EffectContext = MakeEffectContext(GetCurrentAbilitySpecHandle(),GetCurrentActorInfo());
			EffectContext.AddHitResult(Hit);
			DamageSpecHandle.Data->SetContext(EffectContext);
			ApplyGameplayEffectSpecToTarget(
				CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, DamageSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitActor));
			IgnoreTargets.Add(HitActor);
			//속성 추가 효과 적용 (상태이상)
			if (ElementInfoRow && ElementInfoRow->ElementEffect)
			{
				FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(ElementInfoRow->ElementEffect, GetAbilityLevel());
				if (SpecHandle.IsValid())
				{
					ApplyGameplayEffectSpecToTarget(
						CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,SpecHandle,UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitActor));
				}
			}
		}
	}
}

void UMAGameplayAbility_SkillBase::ApplyDamageToTargetData(const FGameplayAbilityTargetDataHandle& TargetData)
{
	if (!BaseDamageEffect || !HasAuthority(&CurrentActivationInfo))
		return;
	
	const F_ElementInfoRow* ElementInfoRow = GetActiveElementInfoRow();
	
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(BaseDamageEffect, GetAbilityLevel());
	if (!DamageSpecHandle.IsValid())
		return;
	//유틸리티 모듈 데미지 배율
	if (ActiveUtilityModule)
	{
		ActiveUtilityModule->ModifyDamageEffectSpec(DamageSpecHandle);
	}
	//속성 모듈 데미지 배율
	if (ElementInfoRow && ElementInfoRow->ElementalDamageMultiplier != 1.f)
	{
		DamageSpecHandle.Data->SetSetByCallerMagnitude(ElementalModifierTag,ElementInfoRow->ElementalDamageMultiplier);
	}
	//행동 모듈 데미지 배율
	if (ActiveBehaviorModule)
	{
		DamageSpecHandle.Data->SetSetByCallerMagnitude(BehaviorModifierTag,ActiveBehaviorModule->GetCurrentDamageMultiplier());
	}
	
	TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetActorsFromTargetData(TargetData, 0);
	for (AActor* TargetActor : TargetActors)
	{
		if (TargetActor && !IgnoreTargets.Contains(TargetActor))
		{
			FGameplayAbilityTargetDataHandle SingleTargetHandle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(TargetActor);
			ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, DamageSpecHandle, SingleTargetHandle);
			IgnoreTargets.Add(TargetActor);
			//속성 상태이상 적용
			if (ElementInfoRow->ElementEffect)
			{
				FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(ElementInfoRow->ElementEffect, GetAbilityLevel());
				if (SpecHandle.IsValid())
				{
					ApplyGameplayEffectSpecToTarget(
						CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,SpecHandle,UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(TargetActor));
				}
			}
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
	if (ActiveBehaviorModule)
		CooldownToApply = ActiveBehaviorModule->CooldownDuration;
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
	if (ActiveBehaviorModule)
		CooldownToApply = ActiveBehaviorModule->ShortCoolDownDuration;

	ApplyBehaviorCooldown(CooldownToApply);
	
	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true);
}

void UMAGameplayAbility_SkillBase::ApplyBehaviorCooldown(float CooldownToApply)
{
	if (!CooldownGE || !HasAuthority(&CurrentActivationInfo))
		return;

	float FinalDuration =  CooldownToApply;
	if (ActiveUtilityModule)
	{
		FinalDuration = ActiveUtilityModule->ModifyCooldownDuration(FinalDuration);
	}
	if (FinalDuration <= 0)
		return;
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE, GetAbilityLevel());
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(CooldownDurationTag, FinalDuration);
		ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);
	}
}
