// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Modules/SkillModule_Instant.h"

#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "GAS/Ability/MAGameplayAbility_Skill.h"
#include "GAS/Projectile/MATargetActor_ChargeAtTarget.h"
#include "GAS/Projectile/MATargetActor_SelectLoc.h"

void USkillModule_Instant::OnAbilityActivated()
{
	if (!OwnerSkill)	return;

	const FSkillData& SkillData = OwnerSkill->GetSkillData();
	if (!SkillData.SkillMontage)
	{
		OwnerSkill->EndAbility(OwnerSkill->GetCurrentAbilitySpecHandle(), OwnerSkill->GetCurrentActorInfo(), OwnerSkill->GetCurrentActivationInfo(), true, false);
		return;
	}
	CachedTargetData = FGameplayAbilityTargetDataHandle();
	
	StartMontageTask();
	
	//즉발 근접 로직
	if (SkillData.ActionTags.HasTag(MeleeActionTag))
	{
		StartWaitDamageEventTask(MontageDamageTag);
	}
	//즉발 투사체 로직
	if (SkillData.ActionTags.HasTag(ProjectileActionTag))
	{
		StartWaitDamageEventTask(MontageSpawnProjectileTag);
	}
	//즉발 타게팅 로직
	if (SkillData.ActionTags.HasTag(TargetingActionTag))
	{
		if (UAnimInstance* AnimInst = OwnerSkill->GetOwnerAnimInstance())
		{
			AnimInst->Montage_SetNextSection(FName("Aiming"), FName("Aiming"),SkillData.SkillMontage);
		}
		StartWaitDamageEventTask(MontageSpawnProjectileTag);
		StartWaitTargetDataTask();
	}
}

void USkillModule_Instant::OnAbilityEnded(bool bWasCancelled)
{
	if (MontageTask)			MontageTask->EndTask();
	if (DamageEventTask)		DamageEventTask->EndTask();
	if (WaitTargetDataTask)		WaitTargetDataTask->EndTask();

	DestroyRangeActor();
}

void USkillModule_Instant::StartMontageTask()
{
	if (!OwnerSkill)	return;
	const FSkillData& SkillData = OwnerSkill->GetSkillData();

	float PlayRate = OwnerSkill->GetTotalAnimSpeed();

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(OwnerSkill,NAME_None,SkillData.SkillMontage,PlayRate,NAME_None,false);
	MontageTask->OnCompleted.AddDynamic(this, &USkillModule_Instant::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &USkillModule_Instant::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &USkillModule_Instant::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &USkillModule_Instant::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void USkillModule_Instant::OnMontageEnded()
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

void USkillModule_Instant::StartWaitDamageEventTask(FGameplayTag EventTag)
{
	if (!OwnerSkill)	return;

	DamageEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwnerSkill,EventTag,nullptr,false,true);
	DamageEventTask->EventReceived.AddDynamic(this, &USkillModule_Instant::OnDamageEventReceived);
	DamageEventTask->ReadyForActivation();
}

void USkillModule_Instant::OnDamageEventReceived(FGameplayEventData Payload)
{
	if (OwnerSkill)
	{
		if (CachedTargetData.IsValid(0))
		{
			Payload.TargetData = CachedTargetData;
		}
		OwnerSkill->ExecuteSkillAction(Payload, 1.f);
	}
}

void USkillModule_Instant::StartWaitTargetDataTask()
{
	if (!OwnerSkill)
		return;

	const FSkillData& SkillData = OwnerSkill->GetSkillData();
	const FActionConfig_Targeting* TargetConfig = SkillData.ActionData.GetPtr<FActionConfig_Targeting>();
	if (!TargetConfig || !TargetConfig->TargetActorClass || !TargetConfig->SkinData)
	{
		OwnerSkill->EndAbility(OwnerSkill->GetCurrentAbilitySpecHandle(), OwnerSkill->GetCurrentActorInfo(), OwnerSkill->GetCurrentActivationInfo(), true, false);
		return;
	}

	if (TargetConfig->RangeActorClass)
	{
		DestroyRangeActor();
		AActor* Avatar = OwnerSkill->GetAvatarActorFromActorInfo();
		if (Avatar && OwnerSkill->GetCurrentActorInfo()->IsLocallyControlled())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = Avatar;
			SpawnParams.Instigator = Cast<APawn>(Avatar);
			SpawnedRangeActor = GetWorld()->SpawnActor<AMAAbilityRangeActor>(TargetConfig->RangeActorClass, Avatar->GetActorTransform(), SpawnParams);

			if (SpawnedRangeActor)
			{
				SpawnedRangeActor->AttachToActor(Avatar, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				SpawnedRangeActor->SetMaxDistance(TargetConfig->MaxDistance);
			}
		}
	}
	
	WaitTargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(OwnerSkill, NAME_None, EGameplayTargetingConfirmation::UserConfirmed, TargetConfig->TargetActorClass);
	WaitTargetDataTask->ValidData.AddDynamic(this, &USkillModule_Instant::OnTargetDataConfirmed);
	WaitTargetDataTask->Cancelled.AddDynamic(this, &USkillModule_Instant::OnTargetDataCancelled);

	AGameplayAbilityTargetActor* SpawnedActor = nullptr;
	WaitTargetDataTask->BeginSpawningActor(OwnerSkill, TargetConfig->TargetActorClass, SpawnedActor);
	AMATargetActor_ChargeAtTarget* TargetActor = Cast<AMATargetActor_ChargeAtTarget>(SpawnedActor);
	if (TargetActor)
	{
		TargetActor->InitializeFixed(TargetConfig->MaxDistance, TargetConfig->ExplodeRadius);
	}
	WaitTargetDataTask->FinishSpawningActor(OwnerSkill, SpawnedActor);
	WaitTargetDataTask->ReadyForActivation();
}

void USkillModule_Instant::OnTargetDataConfirmed(const FGameplayAbilityTargetDataHandle& Data)
{
	DestroyRangeActor();
	CachedTargetData = Data;

	if (WaitTargetDataTask)
	{
		WaitTargetDataTask->EndTask();
		WaitTargetDataTask = nullptr;
	}
	
	float CastSectionLength = 1.f;
	
	if (OwnerSkill && OwnerSkill->GetOwnerAnimInstance())
	{
		const FSkillData& SkillData = OwnerSkill->GetSkillData();
		UAnimMontage* Montage = SkillData.SkillMontage;

		OwnerSkill->GetOwnerAnimInstance()->Montage_SetNextSection(FName("Aiming"),FName("Cast"), Montage);
		OwnerSkill->Montage_SetSection(FName("Cast"));

		if (Montage)
		{
			int32 CastSectionIndex = Montage->GetSectionIndex(FName("Cast"));
			if (CastSectionIndex != INDEX_NONE)
			{
				CastSectionLength = Montage->GetSectionLength(CastSectionIndex);
			}
		}
	}
	
	UAbilityTask_WaitDelay* FinishTimer = UAbilityTask_WaitDelay::WaitDelay(OwnerSkill, CastSectionLength-0.2f);
	FinishTimer->OnFinish.AddDynamic(this, &USkillModule_Instant::OnMontageEnded);
	FinishTimer->ReadyForActivation();
}

void USkillModule_Instant::OnTargetDataCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
	if (WaitTargetDataTask)
	{
		WaitTargetDataTask->EndTask();
		WaitTargetDataTask = nullptr;
	}
	
	DestroyRangeActor();
	if (OwnerSkill)
	{
		if (UAnimInstance* AnimInst = OwnerSkill->GetOwnerAnimInstance())
		{
			if (OwnerSkill->GetCurrentMontage())
			{
				AnimInst->Montage_Stop(0.2f, OwnerSkill->GetCurrentMontage());
			}
		}
		OwnerSkill->EndAbility(OwnerSkill->GetCurrentAbilitySpecHandle(), OwnerSkill->GetCurrentActorInfo(), OwnerSkill->GetCurrentActivationInfo(), true, false);
	}
}

void USkillModule_Instant::DestroyRangeActor()
{
	if (SpawnedRangeActor)
	{
		SpawnedRangeActor->Destroy();
		SpawnedRangeActor = nullptr;
	}
}

