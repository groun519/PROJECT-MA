// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/MAGameplayAbility_Skill.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Character/MACharacter.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MASkillVFXSet.h"
#include "GAS/Modules/MASkillModule.h"
#include "GAS/Modules/SkillModule_Elemental.h"
#include "GAS/Modules/SkillModule_Utility.h"
#include "GAS/Projectile/MAProjectile.h"
#include "GAS/Projectile/MAProjectile_GroundTargetedAOE.h"
#include "GAS/Setting/MASkillSubsystem.h"

UMAGameplayAbility_Skill::UMAGameplayAbility_Skill()
{
	VFXRootTag = FGameplayTag::RequestGameplayTag("Event.VFX");
	IgnoreClearTag = FGameplayTag::RequestGameplayTag("Ability.Combo.Clear");
}

void UMAGameplayAbility_Skill::ActivateAbility(const FGameplayAbilitySpecHandle Handle,	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!LoadSkillData())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true,true);
		return;
	}
	IgnoreTargets.Empty();

	WaitVFXEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, VFXRootTag, nullptr, false,false);
	WaitVFXEventTask->EventReceived.AddDynamic(this, &UMAGameplayAbility_Skill::HandleVFXSpawnEvent);
	WaitVFXEventTask->ReadyForActivation();

	WaitClearEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,IgnoreClearTag);
	WaitClearEventTask->EventReceived.AddDynamic(this, &UMAGameplayAbility_Skill::TargetClear);
	WaitClearEventTask->ReadyForActivation();
	
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
	if (WaitVFXEventTask)
	{
		WaitVFXEventTask->EndTask();
	}
	if (WaitClearEventTask)
	{
		WaitClearEventTask->EndTask();
	}
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
	FActiveGameplayEffectHandle EffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
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

	for (UMASkillModule* Module : ActiveModules)
	{
		if (Module)
		{
			Module->ModifyDamageSpec(SpecHandle);
		}
	}
	return SpecHandle;
}

void UMAGameplayAbility_Skill::ApplyDamageToHitResults(const TArray<FHitResult>& HitResults, float BehaviorMultiplier)
{
	if (!HasAuthority(&CurrentActivationInfo))
		return;
	
	FGameplayEffectSpecHandle MainSpecHandle = MakeSkillDamageSpec(BehaviorMultiplier);
	if (!MainSpecHandle.IsValid())
		return;
	
	// 추가 상태이상 부여
	TArray<FGameplayEffectSpecHandle> AdditionalSpecs;
	for (UMASkillModule* Module : ActiveModules)
	{
		if (Module)
		{
			Module->CreateAdditionalEffectSpecs(AdditionalSpecs);
		}
	}

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
		}
		for (const auto& AddSpec : AdditionalSpecs)
		{
			if (AddSpec.IsValid())
			{
				FGameplayEffectContextHandle AddContext = MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
				AddContext.AddHitResult(Hit);
				AddSpec.Data->SetContext(AddContext);
				
				ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), AddSpec, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitActor));
			}
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

	TArray<FGameplayEffectSpecHandle> AdditionalSpecs;
	for (UMASkillModule* Module : ActiveModules)
	{
		if (Module)
		{
			Module->CreateAdditionalEffectSpecs(AdditionalSpecs);
		}
	}
	
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

			for (const auto& AddSpec : AdditionalSpecs)
			{
				if (AddSpec.IsValid())
				{
					FGameplayEffectContextHandle AddContext = MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
					AddSpec.Data->SetContext(AddContext);
					ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(),GetCurrentActorInfo(),GetCurrentActivationInfo(), AddSpec, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitActor));
				}
			}
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

	bool bIsDamageEvent = Payload.EventTag == FGameplayTag::RequestGameplayTag("Event.Montage.Damage");
	bool bIsProjectileEvent = Payload.EventTag == FGameplayTag::RequestGameplayTag("Event.Montage.SpawnProjectile");

	if (SkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Melee")))
	{
		if (!Payload.EventTag.IsValid() || bIsDamageEvent)
		{
			PerformMeleeAttack(Payload, FinalMultiplier);
		}
	}
	if (SkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Projectile")))
	{
		if (bIsProjectileEvent)
		{
			SpawnProjectile(Payload, FinalMultiplier);
		}
		else if (bIsDamageEvent && !SkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Melee")))
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
	if (!HasAuthority(&CurrentActivationInfo))
		return;
	
	const FSkillData& SkillData = GetSkillData();
	const FActionConfig_Projectile* ProjectileConfig = SkillData.ActionData.GetPtr<FActionConfig_Projectile>();
	if (!ProjectileConfig || !ProjectileConfig->ProjectileClass)
		return;

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
		return;

	FVector AvatarLoc = AvatarActor->GetActorLocation();
	FRotator AvatarRotator = AvatarActor->GetActorRotation();
	int32 Num = FMath::Max(1, ProjectileConfig->NumOfProjectiles);

	float FinalDamageMultiplier = DamageMultiplier * ProjectileConfig->DamageMultiplierPerProjectile;

	for (int32 i=0 ; i<Num ; i++)
	{
		float CurrentAngle = 0.f;
		if (Num >1)
		{
			if (ProjectileConfig->bIsRadial)
			{
				float Step = 360.f / Num;
				CurrentAngle = i*Step;
			}
			else
			{
				float HalfAngle = ProjectileConfig ->SpreadAngle /2.f;
				float Step = ProjectileConfig -> SpreadAngle / (Num -1);
				CurrentAngle = -HalfAngle + (i*Step);
			}
		}
		CurrentAngle += ProjectileConfig ->AngleOffset;

		FRotator SpawnRot = AvatarRotator + FRotator(0.f, CurrentAngle, 0.f);
		FVector SpawnDirection = SpawnRot.Vector();
		FVector SpawnLoc = AvatarLoc + (SpawnDirection * ProjectileConfig->SpawnDistanceFromCharacter);
		
		SpawnProjectileActor(ProjectileConfig->ProjectileClass, SpawnLoc, SpawnRot, FinalDamageMultiplier, ProjectileConfig->ExplodeRadius, ProjectileConfig->bIsPenetrating);
	}
}

void UMAGameplayAbility_Skill::SpawnTargetingProjectile(FGameplayEventData& Payload, float DamageMultiplier)
{
	if (!HasAuthority(&CurrentActivationInfo))
		return;
	
	const FSkillData& SkillData = GetSkillData();
	const FActionConfig_Targeting* TargetConfig = SkillData.ActionData.GetPtr<FActionConfig_Targeting>();

	if (!TargetConfig || !TargetConfig->ProjectileClass)
		return;

	FVector TargetLoc = FVector::ZeroVector;
	if (Payload.TargetData.Num() > 0)
	{
		const FGameplayAbilityTargetData* Data = Payload.TargetData.Get(0);
		if (Data)
		{
			const FHitResult* Hit = Data->GetHitResult();
			if (Hit)
				TargetLoc = Hit->ImpactPoint;
			else
			{
				TargetLoc = Data->GetEndPoint();
			}
		}
	}
	else
	{
		if (AActor* Avatar = GetAvatarActorFromActorInfo())
		{
			TargetLoc = Avatar->GetActorLocation() + (Avatar->GetActorForwardVector()*300.f);
		}
	}

	int32 Num = FMath::Max(1, TargetConfig->NumOfProjectiles);
	float FinalDamageMultiplier = DamageMultiplier * TargetConfig->DamageMultiplierPerProjectile;

	for (int32 i=0 ; i<Num ; i++)
	{
		FVector SpawnLoc = TargetLoc + FVector(0,0,TargetConfig->SpawnHeight);
		FRotator SpawnRot = FRotator(-90.f, 0.f, 0.f);

		if (Num > 1 && TargetConfig->ExplodeRadius > 0.f)
		{
			FVector2D RandPoint = FMath::RandPointInCircle(TargetConfig->ExplodeRadius);
			SpawnLoc += FVector(RandPoint.X, RandPoint.Y, 0.f);
		}
		if (K2_HasAuthority())
		{
			SpawnProjectileActor(TargetConfig->ProjectileClass, SpawnLoc, SpawnRot, FinalDamageMultiplier,TargetConfig->ExplodeRadius);
		}
	}
}

AActor* UMAGameplayAbility_Skill::SpawnProjectileActor(TSubclassOf<AActor> Class, FVector Loc, FRotator Rot,float DamageMultiplier, float ExplodeRadius, bool bIsPenetrating)
{
	if (!Class)
		return nullptr;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetAvatarActorFromActorInfo();
	SpawnParams.Instigator = Cast<APawn>(SpawnParams.Owner);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(Class, Loc, Rot, SpawnParams);
	if (AMAProjectile* Projectile = Cast<AMAProjectile>(SpawnedActor))
	{
		FGameplayEffectSpecHandle SpecHandle = MakeSkillDamageSpec(DamageMultiplier);
		if (SpecHandle.IsValid())
		{
			Projectile->InitializeProjectile(SpecHandle, ExplodeRadius, bIsPenetrating);
		}
	}
	return SpawnedActor;
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

		bool bIsCompatible = BehaviorRow->RequiredTraits.IsEmpty() || CachedSkillData.SkillTraits.HasAll(BehaviorRow->RequiredTraits);

		if (bIsCompatible)
		{
			CachedBehaviorData = *BehaviorRow;
			UMASkillModule* NewModule = NewObject<UMASkillModule>(this, BehaviorRow->ModuleClass);
			if (NewModule)
			{
				NewModule->InitializeModule(this);
				NewModule->ApplyModuleToSkillData(CachedSkillData, *BehaviorRow);
				ActiveModules.Add(NewModule);
			}
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

void UMAGameplayAbility_Skill::TargetClear(FGameplayEventData Payload)
{
	IgnoreTargets.Empty();
}

void UMAGameplayAbility_Skill::HandleVFXSpawnEvent(FGameplayEventData Payload)
{
	if (!HasAuthority(&CurrentActivationInfo))
		return;
	if (!CachedSkillData.VFXDataSet)
		return;

	bool bHasMeleeTrait = CachedSkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Melee"));
	if (!bHasMeleeTrait)
		return;
	
	const F_SkillVFX_Info* VFXInfo = CachedSkillData.VFXDataSet->VFXDataMap.Find(Payload.EventTag);
	if (!VFXInfo || !VFXInfo->DefaultVFX)
		return;

	FLinearColor SpawnColor = FLinearColor::White;
	bool bApplyColor = false;
	if (VFXInfo->bUseElementColor)
	{
		SpawnColor = CachedElementalData.EffectColor;
		bApplyColor = true;
	}
	AMACharacter* Character = Cast<AMACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
		return;
	USkeletalMeshComponent* MeshComp = Character->GetMesh();
	if (!MeshComp)
		return;

	if (VFXInfo->bSpawnInWorld)
	{
		FTransform SocketTransform = (VFXInfo->SocketName != NAME_None)? MeshComp->GetSocketTransform(VFXInfo->SocketName) : MeshComp->GetComponentTransform();
		FTransform OffsetTransform(VFXInfo->RotationOffset, VFXInfo->LocationOffset, VFXInfo->Scale);
		FTransform WorldSPawnTransform = OffsetTransform * SocketTransform;

		Character->Multicast_PlayNiagara(VFXInfo->DefaultVFX, WorldSPawnTransform, bApplyColor, SpawnColor);
	}
	else
	{
		Character->Multicast_PlayNiagaraAttached(VFXInfo->DefaultVFX,VFXInfo->SocketName,VFXInfo->LocationOffset,VFXInfo->RotationOffset,VFXInfo->Scale,	VFXInfo->bAutoDestroy,bApplyColor, SpawnColor);
	}
}
