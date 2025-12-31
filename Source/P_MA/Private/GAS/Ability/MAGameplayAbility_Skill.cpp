// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/MAGameplayAbility_Skill.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/Modules/MASkillModule.h"
#include "GAS/Modules/SkillModule_Elemental.h"
#include "GAS/Modules/SkillModule_Utility.h"
#include "GAS/Setting/MASkillSubsystem.h"

void UMAGameplayAbility_Skill::ActivateAbility(const FGameplayAbilitySpecHandle Handle,	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!LoadSkillData())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true,true);
		return;
	}

	for (UMASkillModule* Module : ActiveModules)
	{
		if (Module)
		{
			Module->OnAbilityActivated();
		}
	}
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UMAGameplayAbility_Skill::EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,bool bReplicateEndAbility, bool bWasCancelled)
{
	for (UMASkillModule* Module : ActiveModules)
	{
		if (Module)
		{
			Module->OnAbilityEnded(bWasCancelled);
		}
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMAGameplayAbility_Skill::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (!CooldownGE)	return;

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
	if (!SpecHandle.IsValid())	return;

	for (const auto& Module : ActiveModules)
	{
		if (Module)
		{
			Module->ModifyCooldownSpec(SpecHandle);
		}
	}
	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
}

void UMAGameplayAbility_Skill::HandleGameplayEvent(FGameplayTag EventTag, FGameplayEventData Payload)
{
	for (UMASkillModule* Module : ActiveModules)
	{
		if (Module)
		{
			Module->OnGameplayEvent(EventTag, Payload);
		}
	}
}

float UMAGameplayAbility_Skill::GetTotalAnimSpeed() const
{
	float TotalSpeed = 1.f;
	for (const auto& Module : ActiveModules)
	{
		if (Module)
		{
			TotalSpeed *= Module->GetAnimSpeedMultiplier();
		}
	}
	return TotalSpeed;
}

void UMAGameplayAbility_Skill::ApplyDamageToHitResults(const TArray<FHitResult>& HitResults)
{
	if (!DamageEffectClass || HitResults.Num() == 0)	return;

	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass,GetAbilityLevel());
	TArray<FGameplayEffectSpecHandle> AdditionalSpecs;
	
	if (!DamageSpecHandle.IsValid())	return;

	for (UMASkillModule* Module : ActiveModules)
	{
		if (Module)
		{
			Module->ModifyDamageSpec(DamageSpecHandle);
			Module->CreateAdditionalEffectSpecs(AdditionalSpecs);
		}
	}

	for (const FHitResult& Hit : HitResults)
	{
		FGameplayAbilityTargetDataHandle TargetDataHandle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(Hit);
		K2_ApplyGameplayEffectSpecToTarget(DamageSpecHandle, TargetDataHandle);

		for (const FGameplayEffectSpecHandle& AddSpec : AdditionalSpecs)
		{
			if (AddSpec.IsValid())
			{
				K2_ApplyGameplayEffectSpecToTarget(AddSpec, TargetDataHandle);
			}
		}
	}
}

bool UMAGameplayAbility_Skill::LoadSkillData()
{
	UMASkillSubsystem* SkillSys = GetWorld()->GetGameInstance()->GetSubsystem<UMASkillSubsystem>();

	if (!SkillSys || SkillID.IsNone())
		return false;

	const FSkillData* SkillRow = SkillSys->GetSkillData(SkillID);
	if (!SkillRow)
		return false;

	CachedSkillData = *SkillRow;
	ActiveModules.Empty();

	const FModuleBehaviorData* BehaviorRow = SkillSys->GetBehaviorData(CachedSkillData.DefaultBehaviorTag);
	if (BehaviorRow && BehaviorRow->ModuleClass)
	{
		CachedBehaviorData = *BehaviorRow;
		UMASkillModule* NewModule = NewObject<UMASkillModule>(this, BehaviorRow->ModuleClass);
		if (NewModule)
		{
			NewModule->InitializeModule(this);
			ActiveModules.Add(NewModule);
		}
	}

	const FModuleElementalData* ElementalRow = SkillSys->GetElementalData(CachedSkillData.DefaultElementalTag);
	if (ElementalRow)
	{
		CachedElementalData = *ElementalRow;
		USkillModule_Elemental* NewModule = NewObject<USkillModule_Elemental>(this, USkillModule_Elemental::StaticClass());
		if (NewModule)
		{
			NewModule->InitializeModule(this);
			ActiveModules.Add(NewModule);
		}
	}

	const FModuleUtilityData* UtilityRow = SkillSys->GetUtilityData(CachedSkillData.DefaultUtilityTag);
	if (UtilityRow)
	{
		CachedUtilityData = *UtilityRow;
		USkillModule_Utility* NewModule = NewObject<USkillModule_Utility>(this, USkillModule_Utility::StaticClass());
		if (NewModule)
		{
			NewModule->InitializeModule(this);
			ActiveModules.Add(NewModule);
		}
	}
	return true;
}
