// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Modules/SkillModule_Charge.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/Modules/MASkillModuleData.h"
#include "GAS/Ability/MAGameplayAbility_Skill.h"
#include "GAS/Projectile/MATargetActor_ChargeAtFwd.h"
#include "GAS/Projectile/MATargetActor_ChargeAtTarget.h"

void USkillModule_Charge::OnAbilityActivated()
{
	if (!OwnerSkill)	return;

	const FSkillData& SkillData = OwnerSkill->GetSkillData();
	if (!SkillData.SkillMontage)
	{
		OwnerSkill->EndAbility(OwnerSkill->GetCurrentAbilitySpecHandle(), OwnerSkill->GetCurrentActorInfo(), OwnerSkill->GetCurrentActivationInfo(), true, false);
		return;
	}
	
	FinalChargedDuration = 0.f;
	bIsCharging = true;

	CachedMaxChargeDuration = 3.f;
	CachedMaxInputDelay = 3.5f;
	
	const FModuleBehaviorData& BehaviorData = OwnerSkill->GetBehaviorData();
	if (const FBehavior_Charge* Config = BehaviorData.ModuleConfig.GetPtr<FBehavior_Charge>())
	{
		CachedMaxChargeDuration = Config->MaxChargeDuration;
		CachedMaxInputDelay = Config->MaxInputDelay;
	}
	
	// 1.몽타주 재생
	StartMontageTask();
	// 2.몽타주 속도 늦춤
	StartChargeTask();
	// 3. 키 입력 해제 대기
	StartWaitInputReleaseTask();
	// 3. 최대 차징 대기
	StartMaxChargeDelayTask();

	//차징 근접 공격 로직
	if (SkillData.ActionTags.HasTag(MeleeActionTag))
	{
		StartWaitDamageEventTask(MontageDamageTag);
	}
	//차징 투사체 로직
	if (SkillData.ActionTags.HasTag(ProjectileActionTag))
	{
		StartWaitDamageEventTask(MontageSpawnProjectileTag);
	}
	//차징 타게팅 로직
	if (SkillData.ActionTags.HasTag(TargetingActionTag))
	{
		StartWaitTargetDataTask();
	}
}

void USkillModule_Charge::OnAbilityEnded(bool bWasCancelled)
{
	if (MontageTask)			MontageTask->EndTask();
	if (InputReleaseTask)		InputReleaseTask->EndTask();
	if (ChargeStartEventTask)	ChargeStartEventTask->EndTask();
	if (DamageEventTask)		DamageEventTask->EndTask();
	if (MaxChargeTask)			MaxChargeTask->EndTask();
	if (WaitTargetDataTask)		WaitTargetDataTask->EndTask();
	
	DestroyActors();
}

void USkillModule_Charge::StartMontageTask()
{
	if (!OwnerSkill)	return;
	const FSkillData& SkillData = OwnerSkill->GetSkillData();

	float PlayRate = OwnerSkill->GetTotalAnimSpeed();

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(OwnerSkill,NAME_None,SkillData.SkillMontage,PlayRate,NAME_None,false);
	MontageTask->OnCompleted.AddDynamic(this, &USkillModule_Charge::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &USkillModule_Charge::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &USkillModule_Charge::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &USkillModule_Charge::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void USkillModule_Charge::OnMontageEnded()
{
	if (OwnerSkill)
	{
		if (UAnimInstance* AnimInst = OwnerSkill->GetOwnerAnimInstance())
		{
			if (OwnerSkill->GetCurrentMontage())
			{
				AnimInst->Montage_Stop(0.2f, OwnerSkill->GetCurrentMontage());
			}
		}
		if (!OwnerSkill->TryActivateComboModule())
		{
			OwnerSkill->EndAbility(OwnerSkill->GetCurrentAbilitySpecHandle(), OwnerSkill->GetCurrentActorInfo(), OwnerSkill->GetCurrentActivationInfo(), true, false);
		}
	}
}

void USkillModule_Charge::StartChargeTask()
{
	if (!OwnerSkill)	return;
	FGameplayTag Tag = FGameplayTag::RequestGameplayTag("Event.Montage.SlowPlay");

	ChargeStartEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwnerSkill, Tag, nullptr, false, true);
	ChargeStartEventTask -> EventReceived.AddDynamic(this, &USkillModule_Charge::OnChargeEventReceived);
	ChargeStartEventTask -> ReadyForActivation();
}

void USkillModule_Charge::OnChargeEventReceived(FGameplayEventData Payload)
{
	if (!OwnerSkill)	return;
	
	if (UAnimMontage* Montage = OwnerSkill->GetCurrentMontage())
	{
		OwnerSkill->Montage_SetPlayRate(Montage, 0.004f);
	}
	
	FinalChargedDuration = 0.f;
}

void USkillModule_Charge::StartWaitDamageEventTask(FGameplayTag EventTag)
{
	if (!OwnerSkill)	return;

	DamageEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwnerSkill,EventTag);
	DamageEventTask->EventReceived.AddDynamic(this, &USkillModule_Charge::OnDamageEventReceived);
	DamageEventTask->ReadyForActivation();
}

void USkillModule_Charge::OnDamageEventReceived(FGameplayEventData Payload)
{
	if (OwnerSkill)
	{
		if (CachedTargetData.IsValid(0))
		{
			Payload.TargetData = CachedTargetData;
		}
		OwnerSkill->ExecuteSkillAction(Payload, FinalChargedDuration);
	}
}

void USkillModule_Charge::StartWaitInputReleaseTask()
{
	if (!OwnerSkill)	return;
	
	InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(OwnerSkill);
	InputReleaseTask -> OnRelease.AddDynamic(this, &USkillModule_Charge::OnInputReleased);
	InputReleaseTask -> ReadyForActivation();
}

void USkillModule_Charge::OnInputReleased(float TimeHeld)
{
	if (!bIsCharging || !OwnerSkill)	return;
	
	FinalChargedDuration = TimeHeld;

	if (CachedMaxChargeDuration > 0.f)
	{
		OwnerSkill->ChargeRatio = FMath::Clamp(FinalChargedDuration / CachedMaxChargeDuration, 0.f, 1.f);
	}
	if (OwnerSkill && OwnerSkill->GetCurrentMontage())
	{
		OwnerSkill->Montage_SetPlayRate(OwnerSkill->GetCurrentMontage(), 1.0f);
	}
	
	bIsCharging = false;
	if (MaxChargeTask)
		MaxChargeTask->EndTask();

	if (CurrentTargetActor)
	{
		if (Cast<AMATargetActor_ChargeAtFwd>(CurrentTargetActor))
		{
			FinishTargetingTask();
		}
		else if (Cast<AMATargetActor_ChargeAtTarget>(CurrentTargetActor))
		{
			CurrentTargetActor->ConfirmTargeting();
		}
	}
}

void USkillModule_Charge::StartMaxChargeDelayTask()
{
	if (!OwnerSkill)	return;
	
	MaxChargeTask = UAbilityTask_WaitDelay::WaitDelay(OwnerSkill, CachedMaxInputDelay);
	MaxChargeTask -> OnFinish.AddDynamic(this, &USkillModule_Charge::OnMaxCharged);
	MaxChargeTask -> ReadyForActivation();
}

void USkillModule_Charge::OnMaxCharged()
{
	if (!bIsCharging || !OwnerSkill)	return;

	FinalChargedDuration = CachedMaxChargeDuration;
	OwnerSkill->ChargeRatio = 1.f;
	
	if (OwnerSkill && OwnerSkill->GetCurrentMontage())
	{
		OwnerSkill->Montage_SetPlayRate(OwnerSkill->GetCurrentMontage(), 1.0f);
	}

	bIsCharging = false;
	if (InputReleaseTask)
		InputReleaseTask->EndTask();

	if (CurrentTargetActor)
	{
		if (Cast<AMATargetActor_ChargeAtFwd>(CurrentTargetActor))
		{
			FinishTargetingTask();
		}
		else if (Cast<AMATargetActor_ChargeAtTarget>(CurrentTargetActor))
		{
			CurrentTargetActor->ConfirmTargeting();
		}
	}
}

void USkillModule_Charge::StartWaitTargetDataTask()
{
	if (!OwnerSkill)
		return;

	const FSkillData& SkillData = OwnerSkill->GetSkillData();
	const FActionConfig_Targeting* TargetConfig = SkillData.ActionData.GetPtr<FActionConfig_Targeting>();
	if (!TargetConfig || !TargetConfig->TargetActorClass)
	{
		OwnerSkill->EndAbility(OwnerSkill->GetCurrentAbilitySpecHandle(), OwnerSkill->GetCurrentActorInfo(), OwnerSkill->GetCurrentActivationInfo(), true, false);
		return;
	}

	AActor* Avatar = OwnerSkill->GetAvatarActorFromActorInfo();
	if (!Avatar)
		return;

	DestroyActors();
	
	//원형 차징 타게팅을 위한 사거리 표시 액터 소환
	if (TargetConfig->RangeActorClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Avatar;
		SpawnedRangeActor = GetWorld()->SpawnActor<AMAAbilityRangeActor>(TargetConfig->RangeActorClass,Avatar->GetActorTransform(), SpawnParams);
		if (SpawnedRangeActor)
		{
			SpawnedRangeActor->AttachToActor(Avatar, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			SpawnedRangeActor->SetMaxDistance(TargetConfig->MaxDistance);
		}
	}
	
	if (TargetConfig->TargetActorClass->IsChildOf(AMATargetActor_ChargeAtFwd::StaticClass()))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Avatar;
		SpawnParams.Instigator = Cast<APawn>(Avatar);
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CurrentTargetActor =  GetWorld()->SpawnActor<AGameplayAbilityTargetActor>(TargetConfig->TargetActorClass, Avatar->GetActorTransform(), SpawnParams);
		if (AMATargetActor_ChargeAtFwd* FwdActor = Cast<AMATargetActor_ChargeAtFwd>(CurrentTargetActor))
		{
			FwdActor->AttachToActor(Avatar, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			FwdActor->StartTargeting(OwnerSkill);
			FwdActor->Initialize(TargetConfig->MaxDistance,TargetConfig->MinDistance,TargetConfig->SkillWidth,TargetConfig->DecalDepth,CachedMaxChargeDuration);
		}
	}
	else if (TargetConfig->TargetActorClass->IsChildOf(AMATargetActor_ChargeAtTarget::StaticClass()))
	{
		WaitTargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(OwnerSkill,NAME_None,EGameplayTargetingConfirmation::Custom,TargetConfig->TargetActorClass);
		WaitTargetDataTask->ValidData.AddDynamic(this, &USkillModule_Charge::OnTargetDataReady);
		WaitTargetDataTask->Cancelled.AddDynamic(this, &USkillModule_Charge::OnTargetDataCancelled);
		WaitTargetDataTask->ReadyForActivation();

		AGameplayAbilityTargetActor* SpawnedActor = nullptr;
		if (WaitTargetDataTask->BeginSpawningActor(OwnerSkill, TargetConfig->TargetActorClass, SpawnedActor))
		{
			if (AMATargetActor_ChargeAtTarget* TargetActor = Cast<AMATargetActor_ChargeAtTarget>(SpawnedActor))
			{
				TargetActor->StartTargeting(OwnerSkill);
				TargetActor->Initialize(TargetConfig->MaxDistance,TargetConfig->MaxSize,TargetConfig->MinSize,CachedMaxChargeDuration);
			}
			WaitTargetDataTask->FinishSpawningActor(OwnerSkill, SpawnedActor);
			CurrentTargetActor = SpawnedActor;
		}
	}
}

void USkillModule_Charge::FinishTargetingTask()
{
	if (!CurrentTargetActor || !OwnerSkill)
		return;
	
	FGameplayAbilityTargetDataHandle TargetData;

	if (AMATargetActor_ChargeAtFwd* FwdActor = Cast<AMATargetActor_ChargeAtFwd>(CurrentTargetActor))
	{
		TargetData = FwdActor->GetTargetData();
	}
	else if (AMATargetActor_ChargeAtTarget* TargetActor = Cast<AMATargetActor_ChargeAtTarget>(CurrentTargetActor))
	{
		TargetData = TargetActor->GetTargetData();
	}
	
	if (TargetData.Num() > 0)
	{
		OwnerSkill->ApplyDamageToTargetData(TargetData, FinalChargedDuration);
	}
	
	DestroyActors();
}

void USkillModule_Charge::DestroyActors()
{
	if (SpawnedRangeActor)
	{
		SpawnedRangeActor->Destroy();
		SpawnedRangeActor=nullptr;
	}
	if (CurrentTargetActor)
	{
		CurrentTargetActor->Destroy();
		CurrentTargetActor=nullptr;
	}
}

void USkillModule_Charge::OnTargetDataReady(const FGameplayAbilityTargetDataHandle& Data)
{
	if (!OwnerSkill)
		return;

	CachedTargetData = Data;
	StartWaitDamageEventTask(MontageSpawnProjectileTag);

	float CastSectionLength = 1.0f;

	if (OwnerSkill->GetOwnerAnimInstance())
	{
		if (UAnimMontage* Montage = OwnerSkill->GetCurrentMontage())
		{
			OwnerSkill->Montage_SetPlayRate(Montage, 1.0f);

			int32 SectionIndex = Montage->GetSectionIndex(FName("Cast"));
			if (SectionIndex != INDEX_NONE)
			{
				CastSectionLength = Montage->GetSectionLength(SectionIndex);
			}
		}
		OwnerSkill->Montage_SetSection(FName("Cast"));
	}
	UAbilityTask_WaitDelay* FinishTimer = UAbilityTask_WaitDelay::WaitDelay(OwnerSkill, CastSectionLength);
	FinishTimer->OnFinish.AddDynamic(this, &USkillModule_Charge::OnMontageEnded);
	FinishTimer->ReadyForActivation();
	
	DestroyActors();
}

void USkillModule_Charge::OnTargetDataCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
	DestroyActors();
}
