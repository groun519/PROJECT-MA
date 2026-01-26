// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Modules/SkillModule_Charge.h"
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
	bIsCharging = false;

	CachedMaxChargeDuration = 3.f;
	CachedMaxInputDelay = 3.5f;
	
	const FModuleBehaviorData& BehaviorData = OwnerSkill->GetBehaviorData();
	if (const FBehavior_Charge* Config = BehaviorData.ModuleConfig.GetPtr<FBehavior_Charge>())
	{
		CachedMaxChargeDuration = Config->MaxChargeDuration;
		CachedMaxInputDelay = Config->MaxInputDelay;
	}
	
	//몽타주 재생 및 애니메이션 속도 늦추도록
	StartMontageTask();
	StartChargeTask();
	StartWaitInputReleaseTask();
	StartMaxChargeDelayTask();

	//차징 근접 공격 로직
	if (SkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Melee")))
	{
		StartWaitDamageEventTask(FName("Event.Montage.Damage"));
	}
	//차징 투사체 로직
	if (SkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Projectile")))
	{
		StartWaitDamageEventTask(FName("Event.Montage.SpawnProjectile"));
	}
	//차징 타게팅 로직
	if (SkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Targeting")))
	{
		if (UAnimInstance* AnimInst = OwnerSkill->GetOwnerAnimInstance())
		{
			AnimInst->Montage_SetNextSection(FName("Aiming"), FName("Aiming"),SkillData.SkillMontage);
		}
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
	MontageTask->ReadyForActivation();
}

void USkillModule_Charge::OnMontageEnded()
{
	if (OwnerSkill)
	{
		OwnerSkill->EndAbility(OwnerSkill->GetCurrentAbilitySpecHandle(), OwnerSkill->GetCurrentActorInfo(), OwnerSkill->GetCurrentActivationInfo(), true, false);
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
		OwnerSkill->Montage_SetPlayRate(Montage, 0.001f);
	}
	bIsCharging = true;
	FinalChargedDuration = 0.f;
}

void USkillModule_Charge::StartWaitDamageEventTask(FName TagName)
{
	if (!OwnerSkill)	return;
	FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(TagName);

	DamageEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwnerSkill,EventTag,nullptr,false,true);
	DamageEventTask->EventReceived.AddDynamic(this, &USkillModule_Charge::OnDamageEventReceived);
	DamageEventTask->ReadyForActivation();
}

void USkillModule_Charge::OnDamageEventReceived(FGameplayEventData Payload)
{
	if (OwnerSkill)
	{
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
	
	
	FinalChargedDuration = FMath::Clamp(TimeHeld, 0.f, CachedMaxChargeDuration);
	
	if (OwnerSkill && OwnerSkill->GetCurrentMontage())
	{
		OwnerSkill->Montage_SetPlayRate(OwnerSkill->GetCurrentMontage(), 1.0f);
	}
	
	bIsCharging = false;
	if (MaxChargeTask)
		MaxChargeTask->EndTask();

	if (CurrentTargetActor)
	{
		FinishTargetingTask();
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
	if (OwnerSkill && OwnerSkill->GetCurrentMontage())
	{
		OwnerSkill->Montage_SetPlayRate(OwnerSkill->GetCurrentMontage(), 1.0f);
	}

	bIsCharging = false;
	if (InputReleaseTask)
		InputReleaseTask->EndTask();

	if (CurrentTargetActor)
	{
		FinishTargetingTask();
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

	//원형 차징 타게팅을 위한 사거리 표시 액터 소환
	if (TargetConfig->RangeActorClass)
	{
		DestroyActors();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Avatar;
		SpawnedRangeActor = GetWorld()->SpawnActor<AMAAbilityRangeActor>(TargetConfig->RangeActorClass,Avatar->GetActorTransform(), SpawnParams);
		if (SpawnedRangeActor)
		{
			SpawnedRangeActor->AttachToActor(Avatar, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			SpawnedRangeActor->SetMaxDistance(TargetConfig->MaxDistance);
		}
	}

	//타겟 액터 소환
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Avatar;
	SpawnParams.Instigator = Cast<APawn>(Avatar);
	
	CurrentTargetActor = GetWorld()->SpawnActor<AGameplayAbilityTargetActor>(TargetConfig->TargetActorClass, Avatar->GetActorTransform(), SpawnParams);
	if (!CurrentTargetActor)
		return;

	//사각형 차징 타겟 액터인 경우
	if (AMATargetActor_ChargeAtFwd* FwdActor = Cast<AMATargetActor_ChargeAtFwd>(CurrentTargetActor))
	{
		FwdActor->AttachToActor(Avatar, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		FwdActor->StartTargeting(OwnerSkill);
		FwdActor->Initialize(TargetConfig->MaxDistance,TargetConfig->MinDistance,TargetConfig->SkillWidth,TargetConfig->DecalDepth,CachedMaxChargeDuration);
	}
	//원형 차징 타겟 액터인 경우
	if (AMATargetActor_ChargeAtTarget* TargetActor = Cast<AMATargetActor_ChargeAtTarget>(CurrentTargetActor))
	{
		TargetActor->Initialize(TargetConfig->MaxDistance,TargetConfig->MaxSize,TargetConfig->MinSize,CachedMaxChargeDuration);
	}
	bIsCharging=true;
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
