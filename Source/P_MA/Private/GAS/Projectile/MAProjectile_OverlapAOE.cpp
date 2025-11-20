// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Projectile/MAProjectile_OverlapAOE.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/EngineTypes.h"



AMAProjectile_OverlapAOE::AMAProjectile_OverlapAOE()
{

}


void AMAProjectile_OverlapAOE::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (!OtherActor || OtherActor == GetOwner())
		return;
	if (GetTeamAttitudeTowards(*OtherActor) != ETeamAttitude::Hostile)
		return;

	UAbilitySystemComponent* OtherASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (OtherASC)
	{
		DamageAndCue();
		Destroy();
	}
}

void AMAProjectile_OverlapAOE::ShootProjectile(float InSpeed, float InMaxDist, float InExplodeRange,
	FGenericTeamId InTeamId, FGameplayEffectSpecHandle InHitEffectHandle)
{
	Super::ShootProjectile(InSpeed, InMaxDist, InExplodeRange, InTeamId, InHitEffectHandle);
	float TravelMaxTime = InMaxDist / InSpeed;
	GetWorld()->GetTimerManager().SetTimer(ShootTimerHandle, this, &AMAProjectile_OverlapAOE::TravelMaxDistanceReached, TravelMaxTime);
}

void AMAProjectile_OverlapAOE::DamageAndCue()
{
	FHitResult HitResult;
	HitResult.ImpactPoint = GetActorLocation();
	HitResult.ImpactNormal = GetActorForwardVector();
	SendLocalGameplayCue(HitResult);
	ApplyAreaDamage(GetActorLocation(), ExplodeRadius,HitResult);
}

void AMAProjectile_OverlapAOE::TravelMaxDistanceReached()
{
	DamageAndCue();
	Destroy();
}
