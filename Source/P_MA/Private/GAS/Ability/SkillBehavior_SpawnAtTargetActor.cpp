// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/SkillBehavior_SpawnAtTargetActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/MACharacter.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "GAS/Projectile/MATargetActor_SelectLoc.h"
#include "GAS/Projectile/MAAbilityRangeActor.h"
#include "GAS/Projectile/MAProjectile_GroundTargetedAOE.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"

USkillBehavior_SpawnAtTargetActor::USkillBehavior_SpawnAtTargetActor()
{
	
}

void USkillBehavior_SpawnAtTargetActor::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();
	if (!OwningAbility || !Character)
		return;

	if (RangeActorClass)
	{
		SpawnedRangeActor = GetWorld()->SpawnActor<AMAAbilityRangeActor>(RangeActorClass);
		if (SpawnedRangeActor)
		{
			SpawnedRangeActor->AttachToActor(Character, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			SpawnedRangeActor->SetAbilityRange(MaxRange);
		}
	}
	
	WaitTargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(OwningAbility, NAME_None, EGameplayTargetingConfirmation::UserConfirmed, TargetActorClass);
	WaitTargetDataTask -> ValidData.AddDynamic(this, &USkillBehavior_SpawnAtTargetActor::TargetConfirmed);
	WaitTargetDataTask -> Cancelled.AddDynamic(this, &USkillBehavior_SpawnAtTargetActor::TargetCancelled);
	WaitTargetDataTask -> ReadyForActivation();
	
	AGameplayAbilityTargetActor* TargetActor;
	WaitTargetDataTask -> BeginSpawningActor(OwningAbility, TargetActorClass, TargetActor);
	AMATargetActor_SelectLoc* SelectLoc = Cast<AMATargetActor_SelectLoc>(TargetActor);
	if (SelectLoc)
	{
		SelectLoc -> SetTargetAreaRadius(AbilitySize);
		SelectLoc -> SetTargetTraceRange(MaxRange);
	}
	WaitTargetDataTask -> FinishSpawningActor(OwningAbility, TargetActor);
}

void USkillBehavior_SpawnAtTargetActor::OnEndAbility_Implementation()
{
	if (WaitTargetDataTask.IsValid())
		WaitTargetDataTask->EndTask();
	if (SpawnedRangeActor)
	{
		SpawnedRangeActor->Destroy();
		SpawnedRangeActor = nullptr;
	}
	
	Super::OnEndAbility_Implementation();
}



void USkillBehavior_SpawnAtTargetActor::TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data)
{
	FVector TargetPoint;
	if (Data.Num() >0 && Data.Get(0)->GetHitResult())
	{
		TargetPoint = Data.Get(0)->GetHitResult()->ImpactPoint;
	}
	else
	{
		TargetPoint = UAbilitySystemBlueprintLibrary::GetTargetDataEndPoint(Data, 0);
	}

	const FVector FinalSpawnLoc = TargetPoint + FVector(0.f, 0.f, SpawnHeight);
	const FRotator FinalSpawnRot = FRotator(-90.f, 0.f, 0.f);
	
	if (Character && ProjectileClass)
		Character -> Server_SpawnGroundTargetedAoEProjectile(
			ProjectileClass, FinalSpawnLoc, FinalSpawnRot, TargetPoint, AbilitySize,DamageEffect);
	
	// 기본공격 나가는 현상 막기 편법
	if (InputLockEffect && OwningAbility)
	{
		if (UAbilitySystemComponent* ASC = OwningAbility->GetAbilitySystemComponentFromActorInfo())
		{
			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(InputLockEffect, OwningAbility->GetAbilityLevel(), EffectContext);
			if (SpecHandle.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
	

	if (OwningAbility)
	{
		OwningAbility->RequestEndAbility();
	}
}

void USkillBehavior_SpawnAtTargetActor::TargetCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
	OwningAbility->RequestEndAbility();
}

