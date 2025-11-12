// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/SkillBehavior_SpawnActorAtTarget.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"
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
	GetWorld()->GetTimerManager().ClearTimer(SpawnLoopTimer);
	
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
	CleanUp();

	//위치데이터 저장
	if (Data.Num() >0 && Data.Get(0)->GetHitResult())
	{
		CachedTargetPoint = Data.Get(0)->GetHitResult()->ImpactPoint;
	}
	else
	{
		CachedTargetPoint = UAbilitySystemBlueprintLibrary::GetTargetDataEndPoint(Data, 0);
	}
	
	if (OwningAbility->K2_HasAuthority())
	{
		//기본값으로 세팅
		CurrentSpawnRule = &DefaultProjectile;

		FGameplayTag ElementTag = OwningAbility->GetSkillElementTag();
		if (ElementTag.IsValid())
		{
			TArray<FName> TagNames;
			UGameplayTagsManager::Get().SplitGameplayTagFName(ElementTag, TagNames);
			FName AttributeName = TagNames.Last();
			//현재 스킬 속성과 같은 Map있으면 덮어씌움
			const FElementSpawnRule* OverrideProjectile = OverrideProjectiles.Find(AttributeName);
			if (OverrideProjectile)
			{
				CurrentSpawnRule = OverrideProjectile;
			}
		}
		if (!CurrentSpawnRule || CurrentSpawnRule->ProjectileClass == nullptr)
		{
			return;
		}
		
		SpawnedCount =0;

		//딜레이 없거나 1발만 쏘는 경우
		if (CurrentSpawnRule->ProjectileCount <=1 || CurrentSpawnRule->ProjectileSpawnDelay <= 0.f)
		{
			for (int32 i=0 ; i<CurrentSpawnRule->ProjectileCount ; ++i)
			{
				FVector SpawnTarget = CachedTargetPoint;
				if (AbilityRange > 0.f)
				{
					FVector2D Offset = FMath::RandPointInCircle(AbilityRange);
					SpawnTarget += FVector(Offset.X, Offset.Y,0.f);
				}
				SpawnSingleProjectile(CurrentSpawnRule->ProjectileClass, SpawnTarget);
			}
		}
		//2발 이상 쏘는 경우
		else
		{
			OnSpawnLoop();
			GetWorld()->GetTimerManager().SetTimer(SpawnLoopTimer,this,&USkillBehavior_SpawnActorAtTarget::OnSpawnLoop, CurrentSpawnRule->ProjectileSpawnDelay, true);
		}
	}
	if (CooldownGE)
		ApplyCooldownAndEndAbility(CooldownGE);
}

void USkillBehavior_SpawnActorAtTarget::OnSpawnLoop()
{
	if (!OwningAbility || !OwningAbility->K2_HasAuthority())
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnLoopTimer);
		return;
	}

	FVector SpawnTarget = CachedTargetPoint;
	if (AbilityRange > 0.f)
	{
		FVector2D Offset = FMath::RandPointInCircle(AbilityRange);
		SpawnTarget += FVector(Offset.X, Offset.Y, 0.f);
	}
	SpawnSingleProjectile(CurrentSpawnRule->ProjectileClass,SpawnTarget);
	SpawnedCount++;

	if (SpawnedCount >= CurrentSpawnRule->ProjectileCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnLoopTimer);
	}
}

void USkillBehavior_SpawnActorAtTarget::SpawnSingleProjectile(TSubclassOf<AMAProjectile_GroundTargetedAOE> ProjectileClass, const FVector& TargetLocation)
{
	if (!ProjectileClass)
		return;

	const FVector FinalSpawnLoc = TargetLocation + FVector(0.f, 0.f, SpawnHeight);
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

void USkillBehavior_SpawnActorAtTarget::TargetCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
	if (ShortCooldownEffect)
		ApplyCooldownAndEndAbility(ShortCooldownEffect);
}

void USkillBehavior_SpawnActorAtTarget::CleanUp()
{
	if (OwningAbility)
	{
		OwningAbility->GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(UMAAbilitySystemStatics::GetAimingTag());
	}
	if (SpawnedRangeActor)
	{
		SpawnedRangeActor->Destroy();
		SpawnedRangeActor = nullptr;
	}
}
