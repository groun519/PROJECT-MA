// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/MAGameplayAbility_Skill.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Camera/CameraComponent.h"
#include "Character/MACharacter.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Setting/MASkillSubsystem.h"

UMAGameplayAbility_Skill::UMAGameplayAbility_Skill()
{
	AbilityTags.AddTag(UMAAbilitySystemStatics::GetSkillAttackTag());

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UMAGameplayAbility_Skill::ActivateAbility(const FGameplayAbilitySpecHandle Handle,	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!LoadSkillData())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true,true);
		return;
	}
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true,true);
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (!CachedSkillData.bCanMove)
		{
			ASC->AddLooseGameplayTag(UMAAbilitySystemStatics::GetMoveBlockTag());
		}
		if (!CachedSkillData.bCanRotate)
		{
			ASC->AddLooseGameplayTag(UMAAbilitySystemStatics::GetRotationLockTag());
		}
	}

	IgnoreTargets.Empty();
	ChargeRatio = 1.f;
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UMAGameplayAbility_Skill::EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (!CachedSkillData.bCanMove)
		{
			ASC->RemoveLooseGameplayTag(UMAAbilitySystemStatics::GetMoveBlockTag());
		}
		if (!CachedSkillData.bCanRotate)
		{
			ASC->RemoveLooseGameplayTag(UMAAbilitySystemStatics::GetRotationLockTag());
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
	
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(GetBaseCooldownEffect(), GetAbilityLevel());
	if (!SpecHandle.IsValid())	return;

	float Duration = CachedSkillData.BaseCooldown;
	SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Cooldown.Duration")), Duration);

	if (CachedSkillData.CooldownTag.IsValid())
	{
		SpecHandle.Data->DynamicGrantedTags.AddTag(CachedSkillData.CooldownTag);
	}

	float FinalDuration = SpecHandle.Data->GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Cooldown.Duration"), false, 0.f);
	if (FinalDuration <= 0.f)
		return;
	
	FActiveGameplayEffectHandle EffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
}


const FGameplayTagContainer* UMAGameplayAbility_Skill::GetCooldownTags() const
{
	if (CachedSkillData.CooldownTag.IsValid())
	{
		static FGameplayTagContainer CooldownTags;
		CooldownTags.Reset();
		CooldownTags.AddTag(CachedSkillData.CooldownTag);
		return &CooldownTags;
	}
	return Super::GetCooldownTags();
}

TSubclassOf<UGameplayEffect> UMAGameplayAbility_Skill::GetBaseCooldownEffect() const
{
	UMAAbilitySystemComponent* ASC = Cast<UMAAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (ASC && ASC->GetSystemGenerics())
		return ASC->GetSystemGenerics()->GetCooldownEffect();
	return nullptr;
}

/********************************************************************************************/
/*										데미지												*/
/********************************************************************************************/

TSubclassOf<UGameplayEffect> UMAGameplayAbility_Skill::GetBaseDamageEffect() const
{
	UMAAbilitySystemComponent* ASC = Cast<UMAAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (ASC && ASC->GetSystemGenerics())
	{
		return ASC->GetSystemGenerics()->GetDamageEffect();
	}
	return nullptr;
}
FGameplayEffectSpecHandle UMAGameplayAbility_Skill::MakeSkillDamageSpec(float BehaviorMultiplier)
{
	TSubclassOf<UGameplayEffect> DamageGE = GetBaseDamageEffect();
	if (!DamageGE)
		return FGameplayEffectSpecHandle();

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageGE, GetAbilityLevel());
	if (!SpecHandle.IsValid())
		return SpecHandle;

	SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage.BehaviorModifier"), BehaviorMultiplier);

	return SpecHandle;
}

void UMAGameplayAbility_Skill::ApplyDamageToHitResults(const TArray<FHitResult>& HitResults, float BehaviorMultiplier)
{
	if (!HasAuthority(&CurrentActivationInfo))
		return;
	
	FGameplayEffectSpecHandle MainSpecHandle = MakeSkillDamageSpec(BehaviorMultiplier);
	if (!MainSpecHandle.IsValid())
		return;
	
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && !IgnoreTargets.Contains(HitActor))
		{
			FGameplayEffectContextHandle EffectContext = MakeEffectContext(GetCurrentAbilitySpecHandle(),GetCurrentActorInfo());
			EffectContext.AddHitResult(Hit);
			MainSpecHandle.Data->SetContext(EffectContext);
			
			ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), MainSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitActor));
			IgnoreTargets.Add(HitActor);

			ApplyHitStop(HitActor);
		}
	}
}

void UMAGameplayAbility_Skill::ApplyDamageToTargetData(const FGameplayAbilityTargetDataHandle& TargetData,float DamageMultiplier)
{
	if (!HasAuthority(&CurrentActivationInfo))
		return;
	
	FGameplayEffectSpecHandle MainSpecHandle = MakeSkillDamageSpec(DamageMultiplier);
	if (!MainSpecHandle.IsValid())
		return;

	TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetActorsFromTargetData(TargetData,0);
	for (AActor* HitActor : TargetActors)
	{
		if (HitActor && !IgnoreTargets.Contains(HitActor))
		{
			FGameplayEffectContextHandle EffectContext = MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
			EffectContext.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());
			MainSpecHandle.Data->SetContext(EffectContext);
			
			ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(),GetCurrentActorInfo(),GetCurrentActivationInfo(), MainSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitActor));
			IgnoreTargets.Add(HitActor);
		}
	}
}

/********************************************************************************************/
/*										공격 액션											*/
/********************************************************************************************/
void UMAGameplayAbility_Skill::ExecuteSkillAction(FGameplayEventData& Payload, float BehaviorMultiplier)
{
	const FSkillData& SkillData = GetSkillData();
	float FinalMultiplier = SkillData.BaseDamageMultiplier * BehaviorMultiplier;

	bool bIsMeleeDamageEvent = Payload.EventTag == UMAAbilitySystemStatics::GetMontageDamageTag();
	bool bIsProjectileEvent = Payload.EventTag == UMAAbilitySystemStatics::GetMontageProjectileTag();

	//Melee 공격에 Montage.Damage 노티파이만 작동하도록
	if (SkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Melee")))
	{
		if (!Payload.EventTag.IsValid() || bIsMeleeDamageEvent)
		{
			PerformMeleeAttack(Payload, FinalMultiplier);
		}
	}
	//Projectile 공격에는 Montage.SpawnProjectile 노티파이만 작동하도록
	if (SkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Projectile")))
	{
		if (bIsProjectileEvent)
		{
			SpawnProjectile(Payload, FinalMultiplier);
		}
		else if (bIsMeleeDamageEvent && !SkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Melee")))
		{
			SpawnProjectile(Payload, FinalMultiplier);
		}
	}
	if (SkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Targeting")))
	{
		SpawnTargetingProjectile(Payload, FinalMultiplier);
	}
}

void UMAGameplayAbility_Skill::PerformMeleeAttack(FGameplayEventData& Payload, float FinalMultiplier)
{
	if (Payload.TargetData.Num() >0)
	{
		TArray<FHitResult> HitResults = GetHitResultFromVirtualSocketTargetData(Payload.TargetData);
		if (HitResults.Num() > 0)
		{
			ApplyDamageToHitResults(HitResults, FinalMultiplier);
		}
	}
}

void UMAGameplayAbility_Skill::SpawnProjectile(FGameplayEventData& Payload, float DamageMultiplier)
{
	(void)Payload;
	(void)DamageMultiplier;
	// Legacy projectile path disabled during projectile init-param migration.
}

void UMAGameplayAbility_Skill::SpawnTargetingProjectile(FGameplayEventData& Payload, float DamageMultiplier)
{
	(void)Payload;
	(void)DamageMultiplier;
	// Legacy targeting-projectile path disabled during projectile init-param migration.
}

AActor* UMAGameplayAbility_Skill::SpawnProjectileActor(TSubclassOf<AActor> Class, FVector Loc, FRotator Rot, float DamageMultiplier, bool bIsPenetrating)
{
	(void)Class;
	(void)Loc;
	(void)Rot;
	(void)DamageMultiplier;
	(void)bIsPenetrating;
	// Legacy projectile spawn path disabled during projectile init-param migration.
	return nullptr;
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
	return true;
}


/********************************************************************************************/
/*										도우미												*/
/********************************************************************************************/
float UMAGameplayAbility_Skill::GetTotalAnimSpeed() const
{
	return 1.f;
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

void UMAGameplayAbility_Skill::ApplyHitStop(AActor* TargetActor)
{
	if (!CachedSkillData.bUseHitStop || CachedSkillData.HitStopDuration <= 0.f)
		return;

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar || !TargetActor)
		return;

	Avatar->CustomTimeDilation = CachedSkillData.HitStopTimeDilation;
	TargetActor->CustomTimeDilation = CachedSkillData.HitStopTimeDilation;

	UCameraComponent* CameraComp = Avatar->FindComponentByClass<UCameraComponent>();
	float OriginalFOV = 0.f;
	bool bApplyCameraEffect = false;
	
	if (CameraComp && CachedSkillData.HitStopZoomOffset > 0.f && !CameraComp->PostProcessSettings.bOverride_VignetteIntensity)
	{
		OriginalFOV = CameraComp->FieldOfView;
		CameraComp->SetFieldOfView(OriginalFOV - CachedSkillData.HitStopZoomOffset);
		CameraComp->PostProcessSettings.bOverride_VignetteIntensity = true;
		CameraComp->PostProcessSettings.VignetteIntensity = CachedSkillData.HitStopVignette;
		CameraComp->PostProcessSettings.bOverride_SceneFringeIntensity = true;
		CameraComp->PostProcessSettings.SceneFringeIntensity = 1.5f;

		bApplyCameraEffect = true;
	}
	
	TWeakObjectPtr<AActor> WeakAvatar = Avatar;
	TWeakObjectPtr<AActor> WeakTarget = TargetActor;

	FTimerHandle HitStopTimer;
	GetWorld()->GetTimerManager().SetTimer(HitStopTimer, [WeakAvatar, WeakTarget, OriginalFOV, bApplyCameraEffect]()
	{
		if (WeakAvatar.IsValid())
		{
			WeakAvatar->CustomTimeDilation = 1.f;

			if (bApplyCameraEffect)
			{
				UCameraComponent* Cam = WeakAvatar->FindComponentByClass<UCameraComponent>();
				if (Cam && OriginalFOV > 0.f)
				{
					Cam->SetFieldOfView(OriginalFOV);
					Cam->PostProcessSettings.bOverride_VignetteIntensity = false;
					Cam->PostProcessSettings.bOverride_SceneFringeIntensity = false;
				}
			}
		}
		if (WeakTarget.IsValid())
		{
			WeakTarget->CustomTimeDilation = 1.f;
		}
	}, CachedSkillData.HitStopDuration, false);
}
