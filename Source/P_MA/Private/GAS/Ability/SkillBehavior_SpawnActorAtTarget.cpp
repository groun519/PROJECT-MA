// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/SkillBehavior_SpawnActorAtTarget.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "AbilitySystemComponent.h"
#include "Character/MACharacter.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "GAS/Projectile/MATargetActor_SelectLoc.h"
#include "GAS/Projectile/MAAbilityRangeActor.h"
#include "GAS/Projectile/MAProjectile_GroundTargetedAOE.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"

USkillBehavior_SpawnActorAtTarget::USkillBehavior_SpawnActorAtTarget()
{
}

void USkillBehavior_SpawnActorAtTarget::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();
	if (!OwningAbility || !Character)
		return;

	OwningAbility->GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(UMAAbilitySystemStatics::GetAimingTag());
	
	if (RangeActorClass)
	{
		SpawnedRangeActor = GetWorld()->SpawnActor<AMAAbilityRangeActor>(RangeActorClass);
		if (SpawnedRangeActor)
		{
			SpawnedRangeActor->AttachToActor(Character, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			SpawnedRangeActor->SetMaxDistance(MaxDistance);
		}
	}
	
	WaitTargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(OwningAbility, NAME_None, EGameplayTargetingConfirmation::UserConfirmed, TargetActorClass);
	WaitTargetDataTask -> ValidData.AddDynamic(this, &USkillBehavior_SpawnActorAtTarget::TargetConfirmed);
	WaitTargetDataTask -> Cancelled.AddDynamic(this, &USkillBehavior_SpawnActorAtTarget::TargetCancelled);
	WaitTargetDataTask -> ReadyForActivation();
	
	AGameplayAbilityTargetActor* TargetActor;
	WaitTargetDataTask -> BeginSpawningActor(OwningAbility, TargetActorClass, TargetActor);
	AMATargetActor_SelectLoc* SelectLoc = Cast<AMATargetActor_SelectLoc>(TargetActor);
	if (SelectLoc)
	{
		SelectLoc -> SetAbilityRadius(AbilityRange);
		SelectLoc -> SetMaxDistance(MaxDistance);
	}
	WaitTargetDataTask -> FinishSpawningActor(OwningAbility, TargetActor);
}

void USkillBehavior_SpawnActorAtTarget::OnEndAbility_Implementation()
{
	OwningAbility->GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(UMAAbilitySystemStatics::GetAimingTag());
	
	if (WaitTargetDataTask.IsValid())
		WaitTargetDataTask->EndTask();
	if (SpawnedRangeActor)
	{
		SpawnedRangeActor->Destroy();
		SpawnedRangeActor = nullptr;
	}
	
	Super::OnEndAbility_Implementation();
}

void USkillBehavior_SpawnActorAtTarget::TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data)
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

	if (OwningAbility->K2_HasAuthority())
	{
		const FVector FinalSpawnLoc = TargetPoint + FVector(0.f, 0.f, SpawnHeight);
		const FRotator FinalSpawnRot = FRotator(-90.f, 0.f, 0.f);
		const FTransform SpawnTransform(FinalSpawnRot,FinalSpawnLoc);

		AActor* OwnerAvatarActor = OwningAbility->GetAvatarActorFromActorInfo();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerAvatarActor;
		SpawnParams.Instigator = Cast<APawn>(OwnerAvatarActor);
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AMAProjectile_GroundTargetedAOE* Projectile = GetWorld()->SpawnActor<AMAProjectile_GroundTargetedAOE>(
			ProjectileClass, SpawnTransform, SpawnParams);
		if (Projectile)
		{
			Projectile->ShootProjectile(ProjectileSpeed,MaxDistance,AbilityRange,
				OwningAbility->GetOwnerTeamId(),OwningAbility->MakeOutgoingGameplayEffectSpec(DamageEffect));
		}
	}
	
	
	if (CooldownGE)
		ApplyCooldownAndEndAbility(CooldownGE);
}

void USkillBehavior_SpawnActorAtTarget::TargetCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
	OwningAbility->RequestEndAbility();
}


