// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/SkillBehavior_SpawnActorAtTarget.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"
#include "Character/MACharacter.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "GAS/Projectile/MATargetActor_SelectLoc.h"
#include "GAS/Projectile/MAAbilityRangeActor.h"
#include "GAS/Projectile/MAProjectile_GroundTargetedAOE.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "GAS/UtilityModule/UtilityModule.h"

USkillBehavior_SpawnActorAtTarget::USkillBehavior_SpawnActorAtTarget()
{
}

void USkillBehavior_SpawnActorAtTarget::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();
	if (!OwningAbility || !Character)
		return;

	if (!ElementSpawnRuleTable)
		return;

	FGameplayTag ElementTag = OwningAbility->GetSkillElementTag();
	if (ElementTag.IsValid())
	{
		TArray<FName> TagNames;
		UGameplayTagsManager::Get().SplitGameplayTagFName(ElementTag, TagNames);
		FName AttributeName = TagNames.Last();

		CurrentSpawnRule = ElementSpawnRuleTable->FindRow<FElementSpawnRule>(AttributeName, TEXT("Rule Lookup"));
	}
	if (!CurrentSpawnRule)
	{
		CurrentSpawnRule = ElementSpawnRuleTable->FindRow<FElementSpawnRule>("Default", TEXT("Rule Lookup"));
	}
	if (!CurrentSpawnRule)
	{
		OwningAbility->ApplyShortCooldownAndRequestEndAbility();
		return;
	}

	OwningAbility->GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(UMAAbilitySystemStatics::GetAimingTag());
	
	if (RangeActorClass)
	{
		SpawnedRangeActor = GetWorld()->SpawnActor<AMAAbilityRangeActor>(RangeActorClass);
		if (SpawnedRangeActor)
		{
			SpawnedRangeActor->AttachToActor(Character, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			SpawnedRangeActor->SetMaxDistance(CurrentSpawnRule->MaxDistance);
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
		SelectLoc -> SetAbilityRadius(CurrentSpawnRule->AbilityRange);
		SelectLoc -> SetMaxDistance(CurrentSpawnRule->MaxDistance);
	}
	WaitTargetDataTask -> FinishSpawningActor(OwningAbility, TargetActor);
}

void USkillBehavior_SpawnActorAtTarget::OnEndAbility_Implementation()
{
	CleanUp();
	GetWorld()->GetTimerManager().ClearTimer(SpawnLoopTimer);
	
	if (WaitTargetDataTask.IsValid())
		WaitTargetDataTask->EndTask();

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
		SpawnedCount =0;
		OwningAbility->ApplyDefaultCooldownOnce();

		//딜레이 없거나 1발만 쏘는 경우
		if (CurrentSpawnRule->ProjectileCount <=1 || CurrentSpawnRule->ProjectileSpawnDelay <= 0.f)
		{
			for (int32 i=0 ; i<CurrentSpawnRule->ProjectileCount ; ++i)
			{
				FVector SpawnTarget = CachedTargetPoint;
				if (CurrentSpawnRule->AbilityRange > 0.f)
				{
					FVector2D Offset = FMath::RandPointInCircle(CurrentSpawnRule->AbilityRange/2);
					SpawnTarget += FVector(Offset.X, Offset.Y,0.f);
				}
				SpawnSingleProjectile(CurrentSpawnRule->ProjectileClass, SpawnTarget);
			}
			SafeEndAbility();
		}
		//2발 이상 쏘는 경우
		else
		{
			GetWorld()->GetTimerManager().SetTimer(SpawnLoopTimer,this,&USkillBehavior_SpawnActorAtTarget::OnSpawnLoop, CurrentSpawnRule->ProjectileSpawnDelay, true,0.f);
		}
	}
}

void USkillBehavior_SpawnActorAtTarget::OnSpawnLoop()
{
	if (!OwningAbility || !OwningAbility->K2_HasAuthority())
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnLoopTimer);
		return;
	}

	FVector SpawnTarget = CachedTargetPoint;
	if (CurrentSpawnRule->AbilityRange > 0.f)
	{
		FVector2D Offset = FMath::RandPointInCircle(CurrentSpawnRule->AbilityRange/2);
		SpawnTarget += FVector(Offset.X, Offset.Y, 0.f);
	}
	SpawnSingleProjectile(CurrentSpawnRule->ProjectileClass,SpawnTarget);
	SpawnedCount++;

	if (SpawnedCount >= CurrentSpawnRule->ProjectileCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnLoopTimer);
		SafeEndAbility();
	}
}

void USkillBehavior_SpawnActorAtTarget::SpawnSingleProjectile(TSubclassOf<AMAProjectile_GroundTargetedAOE> ProjectileClass, const FVector& TargetLocation)
{
	if (!ProjectileClass)
		return;
	AActor* OwnerAvatarActor = OwningAbility->GetAvatarActorFromActorInfo();
	if (!OwnerAvatarActor)
		return;

	const FVector SpawnLocation = OwnerAvatarActor->GetActorLocation() + FVector(0.f,0.f,CurrentSpawnRule->SpawnHeight);
	const FVector Direction = (TargetLocation - SpawnLocation).GetSafeNormal();
	const FRotator SpawnRotation = Direction.Rotation();
	const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

	const float TravelDistance = (TargetLocation - SpawnLocation).Size();
	const float ProjectileSpeed = TravelDistance/CurrentSpawnRule->TravelTime;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerAvatarActor;
	SpawnParams.Instigator = Cast<APawn>(OwnerAvatarActor);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const F_ElementInfoRow* ElementInfoRow = OwningAbility->GetActiveElementInfoRow();
	FGameplayEffectSpecHandle SpecHandle = OwningAbility->MakeOutgoingGameplayEffectSpec(OwningAbility->GetBaseDamageEffect());

	//유틸리티 데미지 적용
	if (OwningAbility->GetActiveUtilityModule())
	{
		OwningAbility->GetActiveUtilityModule()->ModifyDamageEffectSpec(SpecHandle);
	}
	
	//속성 데미지 적용
	if (ElementInfoRow && ElementInfoRow->ElementalDamageMultiplier != 1.f)
	{
		SpecHandle.Data->SetSetByCallerMagnitude(
			UMAAbilitySystemStatics::GetElementalMultiplierTag(),
			ElementInfoRow->ElementalDamageMultiplier);
	}
	//행동 데미지 배율
	SpecHandle.Data->SetSetByCallerMagnitude(UMAAbilitySystemStatics::GetBehaviorMultiplierTag(),BehaviorDamageMultiplier);

	AMAProjectile_GroundTargetedAOE* Projectile = GetWorld()->SpawnActor<AMAProjectile_GroundTargetedAOE>(
			ProjectileClass, SpawnTransform, SpawnParams);
	if (Projectile)
	{
		if (ElementInfoRow->ElementEffect)
		{	//속성 추가 효과 적용
			Projectile->AdditionalEffect = ElementInfoRow->ElementEffect;
		}
		Projectile->ShootProjectile(ProjectileSpeed,CurrentSpawnRule->MaxDistance,CurrentSpawnRule->AbilityRange,
			OwningAbility->GetOwnerTeamId(),SpecHandle);
	}
}

void USkillBehavior_SpawnActorAtTarget::TargetCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
	CleanUp();
	OwningAbility->ApplyShortCooldownAndRequestEndAbility();
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
