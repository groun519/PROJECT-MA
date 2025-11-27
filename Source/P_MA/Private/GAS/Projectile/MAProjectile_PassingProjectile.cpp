// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Projectile/MAProjectile_PassingProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"

void AMAProjectile_PassingProjectile::NotifyActorBeginOverlap(class AActor* OtherActor)
{
	if (!OtherActor || OtherActor == GetOwner())
		return;
	if (GetTeamAttitudeTowards(*OtherActor) != ETeamAttitude::Hostile)
		return;

	UAbilitySystemComponent* OtherASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (OtherASC)
	{
		if (HitEffectHandle.IsValid())
		{
			OtherASC->ApplyGameplayEffectSpecToSelf(*HitEffectHandle.Data.Get());
		}
		if (AdditionalEffect)
		{
			UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
			if (SourceASC)
			{
				FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
				EffectContext.AddHitResult(FHitResult(OtherActor, nullptr, GetActorLocation(),GetActorForwardVector()));

				FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(AdditionalEffect, 1.f,EffectContext);
				if (SpecHandle.IsValid())
				{
					SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(),OtherASC);
				}
			}
		}
		FGameplayCueParameters CueParams;
		CueParams.Location = GetActorLocation();
		CueParams.Normal = GetActorForwardVector();
		CueParams.Instigator = GetInstigator();
		CueParams.EffectCauser = this;
		OtherASC->ExecuteGameplayCue(HitGameplayCueTag,CueParams);
	}
}

void AMAProjectile_PassingProjectile::ShootProjectile(float InSpeed, float InMaxDist, float InExplodeRange,
	FGenericTeamId InTeamId, FGameplayEffectSpecHandle InHitEffectHandle)
{
	ProjectileSpeed = InSpeed;
	HitEffectHandle = InHitEffectHandle;
	ExplodeRadius = InExplodeRange;
	MoveDir = GetActorRotation().Vector();

	TArray<AActor*> OverlappingActors;
    
	// CollisionComp는 부모인 MAProjectileBase에 정의되어 있다고 가정 (보통 SphereComponent)
	if (CollisionComp)
	{
		CollisionComp->GetOverlappingActors(OverlappingActors);
        
		for (AActor* Actor : OverlappingActors)
		{
			// 이미 겹쳐진 액터들에 대해 강제로 오버랩 로직 실행
			NotifyActorBeginOverlap(Actor);
		}
	}
	
	float TravelMaxTime = InMaxDist / InSpeed;
	GetWorld()->GetTimerManager().SetTimer(ShootTimerHandle,this, &AMAProjectile_PassingProjectile::TravelMaxDistance, TravelMaxTime);
	SetGenericTeamId(InTeamId);
}

void AMAProjectile_PassingProjectile::TravelMaxDistance()
{
	Destroy();
}
