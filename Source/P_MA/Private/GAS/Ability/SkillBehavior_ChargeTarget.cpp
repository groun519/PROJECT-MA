// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/SkillBehavior_ChargeTarget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "MAGameplayAbility_SkillBase.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Character/MACharacter.h"
#include "GAS/Projectile/MATargetActor_ChargeAtTarget.h"

void USkillBehavior_ChargeTarget::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();

	if (!OwningAbility)
		return;

	if (MaxDistanceActorClass)
	{
		DistanceActor = GetWorld()->SpawnActor<AMAAbilityRangeActor>(MaxDistanceActorClass);
		if (DistanceActor)
		{
			DistanceActor->AttachToActor(Character, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			DistanceActor->SetMaxDistance(MaxDistance);
		}
	}

	WaitDelay = UAbilityTask_WaitDelay::WaitDelay(OwningAbility, TimeoutDuration);
	WaitDelay->OnFinish.AddDynamic(this, &USkillBehavior_ChargeTarget::OnDelayFinished);
	WaitDelay->ReadyForActivation();

	WaitInputRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(OwningAbility);
	WaitInputRelease->OnRelease.AddDynamic(this, &USkillBehavior_ChargeTarget::OnReleased);
	WaitInputRelease->ReadyForActivation();
	
	WaitTargetData = UAbilityTask_WaitTargetData::WaitTargetData(OwningAbility,NAME_None,EGameplayTargetingConfirmation::UserConfirmed, TargetActorClass);
	WaitTargetData->ValidData.AddDynamic(this, &USkillBehavior_ChargeTarget::OnConfirmed);
	WaitTargetData->Cancelled.AddDynamic(this, &USkillBehavior_ChargeTarget::OnCancelled);
	WaitTargetData->ReadyForActivation();

	AGameplayAbilityTargetActor* TA;
	WaitTargetData->BeginSpawningActor(OwningAbility, TargetActorClass, TA);
	TargetActor = Cast<AMATargetActor_ChargeAtTarget>(TA);
	if (TargetActor)
	{
		TargetActor->Initialize(MaxDistance, MaxSize, MinSize, MaxHoldDuration);
	}
	WaitTargetData->FinishSpawningActor(OwningAbility, TA);
}

void USkillBehavior_ChargeTarget::OnEndAbility_Implementation()
{
	if (WaitDelay.IsValid())
		WaitDelay->EndTask();
	if (WaitInputRelease.IsValid())
		WaitInputRelease->EndTask();
	if (WaitTargetData.IsValid())
		WaitTargetData->EndTask();

	if (DistanceActor)
	{
		DistanceActor->Destroy();
		DistanceActor=nullptr;
	}
	if (TargetActor)
	{
		TargetActor->Destroy();
		TargetActor=nullptr;
	}
	Super::OnEndAbility_Implementation();
}

void USkillBehavior_ChargeTarget::OnConfirmed(const FGameplayAbilityTargetDataHandle& Data)
{
	if (!TargetActor || !OwningAbility)
		return;
	
	CachedTargetData = Data;
	
	FVector TargetLoc = TargetActor->FinalImpactPoint;
	float ChargeRatio = TargetActor->FinalChargeRatio;
	float FinalSize = FMath::Lerp(MinSize, MaxSize, ChargeRatio);
	SpawnVFX(TargetLoc, FinalSize);
	
	UAbilityTask_WaitDelay* AttackLock = UAbilityTask_WaitDelay::WaitDelay(OwningAbility, 0.05f);
	AttackLock->OnFinish.AddDynamic(this, &USkillBehavior_ChargeTarget::AttackBlcoking);
	AttackLock->ReadyForActivation();
}

void USkillBehavior_ChargeTarget::OnCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
	OwningAbility->RequestEndAbility();
}

void USkillBehavior_ChargeTarget::SpawnVFX(FVector SpawnLoc, float FinalSize)
{
	if (!ExecutionVFX || !Character)
		return;
	FRotator Rotation = FRotator::ZeroRotator;

	float SafeRadius = (VFXRadius == 0.f) ? 1.f : VFXRadius;

	FVector Scale = FVector (FinalSize / SafeRadius);
	FTransform SpawnTransform(Rotation,SpawnLoc,Scale);
	Character->Multicast_PlayNiagara(ExecutionVFX,SpawnTransform);
}

void USkillBehavior_ChargeTarget::OnDelayFinished()
{
	OwningAbility->RequestEndAbility();
}

void USkillBehavior_ChargeTarget::OnReleased(float TimeHeld)
{
	if (OwningAbility->K2_HasAuthority())
	{
		UE_LOG(LogTemp,Warning,TEXT("Target has authority"));
		OwningAbility->ApplyDamageToTargetData(CachedTargetData, DamageEffect);
	}else
	{
		UE_LOG(LogTemp,Warning,TEXT("Target has not authority"));
	}
	OwningAbility->RequestEndAbility();
}

void USkillBehavior_ChargeTarget::AttackBlcoking()
{
	OwningAbility->RequestEndAbility();
}
