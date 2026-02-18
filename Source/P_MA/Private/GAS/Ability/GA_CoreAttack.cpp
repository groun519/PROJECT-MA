// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Ability/GA_CoreAttack.h"
#include "AI/Golem/Monster.h"
#include "AbilitySystemComponent.h"
#include "EngineUtils.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/PA_AbilitySystemGenerics.h"
#include "GAS/Projectile/MAProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"

UGA_CoreAttack::UGA_CoreAttack()
{
	CooldownTag = FGameplayTag::RequestGameplayTag("Ability.Cooldown.CoreAttack");
}

void UGA_CoreAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	AMonster* Target = FindNearestMonster();
	if (bRequireTarget && !Target)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CachedTarget = Target;

	if (bAimToTarget && Target)
	{
		AimAtTarget(Target);
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	SpawnProjectile();
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_CoreAttack::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(GetBaseCooldownEffect(), GetAbilityLevel());
	if (!SpecHandle.IsValid()) return;

	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const float AttackSpeed = FMath::Max(
		MinAttackSpeedForCooldown,
		ASC ? ASC->GetNumericAttribute(UMAAttributeSet::GetAttackSpeedAttribute()) : 1.f);
	const float Duration = BaseCooldown / AttackSpeed;
	SpecHandle.Data->SetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag(FName("Data.Cooldown.Duration")),
		Duration);

	if (CooldownTag.IsValid())
	{
		SpecHandle.Data->DynamicGrantedTags.AddTag(CooldownTag);
	}

	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
}

const FGameplayTagContainer* UGA_CoreAttack::GetCooldownTags() const
{
	if (CooldownTag.IsValid())
	{
		static FGameplayTagContainer CooldownTags;
		CooldownTags.Reset();
		CooldownTags.AddTag(CooldownTag);
		return &CooldownTags;
	}
	return Super::GetCooldownTags();
}

AMonster* UGA_CoreAttack::FindNearestMonster() const
{
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar || TargetingRange <= 0.f) return nullptr;

	const float RangeSq = TargetingRange * TargetingRange;
	AMonster* BestTarget = nullptr;
	float BestDistSq = RangeSq;

	for (TActorIterator<AMonster> It(Avatar->GetWorld()); It; ++It)
	{
		AMonster* Monster = *It;
		if (!IsValid(Monster) || !Monster->IsActive()) continue;

		const float DistSq = FVector::DistSquared(Avatar->GetActorLocation(), Monster->GetActorLocation());
		if (DistSq <= BestDistSq)
		{
			BestDistSq = DistSq;
			BestTarget = Monster;
		}
	}

	return BestTarget;
}

void UGA_CoreAttack::AimAtTarget(const AActor* Target) const
{
	if (!Target) return;

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return;

	const FVector ToTarget = Target->GetActorLocation() - Avatar->GetActorLocation();
	if (ToTarget.IsNearlyZero()) return;

	const FRotator LookRot = ToTarget.Rotation();
	Avatar->SetActorRotation(FRotator(0.f, LookRot.Yaw, 0.f));
}

void UGA_CoreAttack::SpawnProjectile()
{
	if (!HasAuthority(&CurrentActivationInfo)) return;
	if (!ProjectileClass) return;

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return;

	const FVector SpawnDirection = Avatar->GetActorUpVector();
	const FVector SpawnLoc = Avatar->GetActorLocation() + (SpawnDirection * SpawnDistanceFromCharacter);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Avatar;
	SpawnParams.Instigator = Cast<APawn>(Avatar);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLoc, Avatar->GetActorRotation(), SpawnParams);
	if (AMAProjectile* Projectile = Cast<AMAProjectile>(SpawnedActor))
	{
		if (CachedTarget.IsValid())
		{
			Projectile->SetOnlyDamageTarget(CachedTarget.Get());
		}

		FGameplayEffectSpecHandle SpecHandle = MakeDamageSpec();
		if (SpecHandle.IsValid())
		{
			Projectile->InitializeProjectile(SpecHandle, ExplodeRadius, bIsPenetrating);
		}

		if (Projectile->ProjectileMovement)
		{
			ScheduleHomingAndCollision(Projectile);
		}
	}
}

TSubclassOf<UGameplayEffect> UGA_CoreAttack::GetBaseDamageEffect() const
{
	const UMAAbilitySystemComponent* ASC = Cast<UMAAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	const UPA_AbilitySystemGenerics* Generics = ASC ? ASC->GetSystemGenerics() : nullptr;
	return Generics ? Generics->GetDamageEffect() : nullptr;
}

void UGA_CoreAttack::ScheduleHomingAndCollision(AMAProjectile* Projectile)
{
	if (!Projectile || !bUseHoming || !CachedTarget.IsValid() || !Projectile->ProjectileMovement) return;

	if (HomingActivationDelay <= 0.f)
	{
		Projectile->ProjectileMovement->HomingTargetComponent = CachedTarget->GetRootComponent();
		Projectile->ProjectileMovement->bIsHomingProjectile = true;
		Projectile->ProjectileMovement->HomingAccelerationMagnitude = HomingAccelerationMagnitude;
		return;
	}

	if (Projectile->SphereComp)
	{
		Projectile->SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	Projectile->ProjectileMovement->bIsHomingProjectile = false;

	TWeakObjectPtr<AMAProjectile> WeakProjectile = Projectile;
	TWeakObjectPtr<USceneComponent> WeakTargetComp = CachedTarget->GetRootComponent();
	const float Accel = HomingAccelerationMagnitude;

	FTimerHandle TimerHandle;
	Projectile->GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateLambda([WeakProjectile, WeakTargetComp, Accel]()
		{
			AMAProjectile* P = WeakProjectile.Get();
			if (!P) return;
			if (P->SphereComp)
			{
				P->SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			}
			if (P->ProjectileMovement)
			{
				P->ProjectileMovement->HomingTargetComponent = WeakTargetComp.Get();
				P->ProjectileMovement->bIsHomingProjectile = true;
				P->ProjectileMovement->HomingAccelerationMagnitude = Accel;
			}
		}),
		HomingActivationDelay,
		false);
}

FGameplayEffectSpecHandle UGA_CoreAttack::MakeDamageSpec() const
{
	TSubclassOf<UGameplayEffect> DamageGE = GetBaseDamageEffect();
	if (!DamageGE) return FGameplayEffectSpecHandle();

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageGE, GetAbilityLevel());
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(UMAAbilitySystemStatics::GetBehaviorMultiplierTag(), DamageMultiplier);
	}
	return SpecHandle;
}

TSubclassOf<UGameplayEffect> UGA_CoreAttack::GetBaseCooldownEffect() const
{
	const UMAAbilitySystemComponent* ASC = Cast<UMAAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	const UPA_AbilitySystemGenerics* Generics = ASC ? ASC->GetSystemGenerics() : nullptr;
	return Generics ? Generics->GetCooldownEffect() : nullptr;
}
