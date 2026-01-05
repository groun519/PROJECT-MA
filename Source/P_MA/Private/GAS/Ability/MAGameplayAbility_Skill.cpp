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
	IgnoreTargets.Empty();

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

/********************************************************************************************/
/*										쿨타임												*/
/********************************************************************************************/
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

/********************************************************************************************/
/*										데미지												*/
/********************************************************************************************/
void UMAGameplayAbility_Skill::ApplyDamageToHitResults(const TArray<FHitResult>& HitResults, float BehaviorMultiplier)
{
	if (!DamageEffectClass)	return;

	FGameplayEffectSpecHandle MainSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass,GetAbilityLevel());
	if (!MainSpecHandle.IsValid())	return;

	// 행동 모듈 데미지 보정
	MainSpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage.BehaviorModifier"), BehaviorMultiplier);

	// 나머지 모듈 데미지 보정
	TArray<FGameplayEffectSpecHandle> AdditionalSpecs;
	for (UMASkillModule* Module : ActiveModules)
	{
		if (Module)
		{
			Module->ModifyDamageSpec(MainSpecHandle);
			Module->CreateAdditionalEffectSpecs(AdditionalSpecs);
		}
	}

	for (const FHitResult& Hit : HitResults)
	{
		if (AActor* TargetActor = Hit.GetActor())
		{
			if (IgnoreTargets.Contains(TargetActor))
				continue;

			IgnoreTargets.Add(TargetActor);
			FGameplayAbilityTargetDataHandle TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(TargetActor);
			MainSpecHandle.Data->GetContext().AddHitResult(Hit);
	
			ApplyGameplayEffectSpecToTarget(
				GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), MainSpecHandle, TargetData);

			for (const FGameplayEffectSpecHandle& AddSpec : AdditionalSpecs)
			{
				if (AddSpec.IsValid())
				{
					AddSpec.Data->GetContext().AddHitResult(Hit);

					ApplyGameplayEffectSpecToTarget(
						GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), AddSpec, TargetData);
				}
			}
		}
	}
}

void UMAGameplayAbility_Skill::ExecuteSkillAction(FGameplayEventData& Payload, float ChargeLevel)
{
	const FSkillData& SkillData = GetSkillData();
	float FinalMultiplier = SkillData.BaseDamageMultiplier * ChargeLevel;

	if (SkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Melee")))
	{
		PerformMeleeAttack(Payload, FinalMultiplier);
	}
}

void UMAGameplayAbility_Skill::PerformMeleeAttack(FGameplayEventData& Payload, float FinalMultiplier)
{
	if (Payload.TargetData.Num() > 0)
	{
		TArray<FHitResult> HitResults = GetHitResultFromVirtualSocketTargetData(Payload.TargetData);
		if (HitResults.Num() > 0)
		{
			ApplyDamageToHitResults(HitResults, FinalMultiplier);
		}
	}
}

void UMAGameplayAbility_Skill::SpawnProjectile(FGameplayEventData& Payload, float ChargeLevel)
{
	const FSkillData& SkillData = GetSkillData();
	if (!SkillData.ProjectileClass)	return;

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)	return;

	FVector SpawnLoc = AvatarActor->GetActorLocation();
	FRotator SpawnRot = AvatarActor->GetActorRotation();

	if (Payload.TargetData.Num() > 0)
	{
		const FGameplayAbilityTargetData* RawData = Payload.TargetData.Get(0);
		if (RawData && RawData->GetScriptStruct() == FGameplayAbilityTargetData_LocationInfo::StaticStruct())
		{
			const FGameplayAbilityTargetData_LocationInfo* LocInfo = static_cast<const FGameplayAbilityTargetData_LocationInfo*>(RawData);
			SpawnLoc = LocInfo->SourceLocation.GetTargetingTransform().GetLocation();
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = AvatarActor;
	SpawnParams.Instigator = Cast<APawn>(AvatarActor);

	GetWorld()->SpawnActor<AActor>(SkillData.ProjectileClass, SpawnLoc, SpawnRot, SpawnParams);
}


/********************************************************************************************/
/*										초기화												*/
/********************************************************************************************/
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

/********************************************************************************************/
/*										도우미												*/
/********************************************************************************************/
float UMAGameplayAbility_Skill::GetTotalAnimSpeed() const
{
	float TotalSpeed = 1.f;
	for (UMASkillModule* Module : ActiveModules)
	{
		if (Module)
		{
			TotalSpeed *= Module->GetAnimSpeedMultiplier();
		}
	}
	return TotalSpeed;
}

void UMAGameplayAbility_Skill::Montage_SetPlayRate(UAnimMontage* AnimMontage, float PlayRate)
{
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	if (AnimInstance && AnimMontage)
	{
		AnimInstance->Montage_SetPlayRate(AnimMontage, PlayRate);
	}
}

void UMAGameplayAbility_Skill::Montage_SetSection(FName SectionName)
{
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
