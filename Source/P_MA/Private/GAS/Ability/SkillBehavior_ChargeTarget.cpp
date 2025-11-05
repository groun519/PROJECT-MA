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

	PressedTime = GetWorld()->GetTimeSeconds();

	if (MaxDistanceActorClass)
	{
		DistanceActor = GetWorld()->SpawnActor<AMAAbilityRangeActor>(MaxDistanceActorClass);
		if (DistanceActor)
		{
			DistanceActor->AttachToActor(Character, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			DistanceActor->SetMaxDistance(MaxDistance);
		}
	}
	WaitTargetData = UAbilityTask_WaitTargetData::WaitTargetData(OwningAbility, NAME_None,EGameplayTargetingConfirmation::UserConfirmed,TargetActorClass);
	WaitTargetData->ValidData.AddDynamic(this, &USkillBehavior_ChargeTarget::TargetConfirmed);
	WaitTargetData->Cancelled.AddDynamic(this, &USkillBehavior_ChargeTarget::TargetCancelled);
	WaitTargetData->ReadyForActivation();

	AGameplayAbilityTargetActor* TA;
	WaitTargetData->BeginSpawningActor(OwningAbility,TargetActorClass, TA);
	TargetActor = Cast<AMATargetActor_ChargeAtTarget>(TA);
	if (TargetActor)
	{
		TargetActor->Initialize(MaxDistance,MaxSize,MinSize,MaxHoldDuration);
	}
	WaitTargetData->FinishSpawningActor(OwningAbility, TA);

	WaitDelay = UAbilityTask_WaitDelay::WaitDelay(OwningAbility, TimeoutDuration);
	WaitDelay->OnFinish.AddDynamic(this, &USkillBehavior_ChargeTarget::OnDelayFinished);
	WaitDelay->ReadyForActivation();

	WaitInputRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(OwningAbility);
	WaitInputRelease->OnRelease.AddDynamic(this, &USkillBehavior_ChargeTarget::OnReleased);
	WaitInputRelease->ReadyForActivation();
}

void USkillBehavior_ChargeTarget::OnEndAbility_Implementation()
{
	if (WaitDelay.IsValid())
		WaitDelay->EndTask();
	if (WaitInputRelease.IsValid())
		WaitInputRelease->EndTask();

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

void USkillBehavior_ChargeTarget::SpawnVFX(FVector SpawnLoc,float FinalSize)
{
	if (!ExecutionVFX || !Character)
		return;
	FRotator Rotation = FRotator::ZeroRotator;

	float SafeRadius = (VFXRadius == 0.f) ? 1.f : VFXRadius;

	FVector Scale = FVector (FinalSize / SafeRadius);
	FTransform SpawnTransform(Rotation,SpawnLoc,Scale);
	Character->Multicast_PlayNiagara(ExecutionVFX,SpawnTransform);
}

void USkillBehavior_ChargeTarget::TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data)
{
	float HeldTime = GetWorld()->GetTimeSeconds() - PressedTime;

	if (HeldTime <= MinimumTimeToActive) 
	{
		ApplyCooldownAndEndAbility(ShortCooldownEffect);
		return;
	}
	// Data 인덱스 1 : 위치 데이터
	FVector TargetPoint;
	if (Data.Num() > 1 && Data.Get(1)->GetHitResult())
	{
		TargetPoint = Data.Get(1)->GetHitResult()->ImpactPoint;
	}else
	{
		TargetPoint = UAbilitySystemBlueprintLibrary::GetTargetDataEndPoint(Data,1);
	}
	
	float ChargeRatio = FMath::Clamp(HeldTime / MaxHoldDuration, 0.f, 1.f);
	float FinalSize = FMath::Lerp(MinSize, MaxSize, ChargeRatio);

	if (OwningAbility->K2_HasAuthority())
	{
		SpawnVFX(TargetPoint,FinalSize);
		OwningAbility->ApplyDamageToTargetData(Data, DamageEffect);
	}

	ApplyCooldownAndEndAbility(CooldownGE);
}

void USkillBehavior_ChargeTarget::TargetCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
	if (ShortCooldownEffect)
		OwningAbility->ApplyEffectToOwner(ShortCooldownEffect);
	OwningAbility->RequestEndAbility();
}

void USkillBehavior_ChargeTarget::OnDelayFinished()
{
	if (ShortCooldownEffect)
	{
		UAbilityTask_WaitDelay* Fuck = UAbilityTask_WaitDelay::WaitDelay(OwningAbility, 0.05f);
		Fuck->OnFinish.AddDynamic(this, &USkillBehavior_ChargeTarget::SafeEndAbility);
		Fuck->ReadyForActivation();
		OwningAbility->ApplyEffectToOwner(ShortCooldownEffect);
	}
}

void USkillBehavior_ChargeTarget::OnReleased(float TimeHeld)
{
	if (ShortCooldownEffect)
		OwningAbility->ApplyEffectToOwner(ShortCooldownEffect);
	OwningAbility->RequestEndAbility();
}