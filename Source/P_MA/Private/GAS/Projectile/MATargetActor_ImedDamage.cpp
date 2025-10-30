// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Projectile/MATargetActor_ImedDamage.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/GameplayAbility.h"
#include "Components/DecalComponent.h"
#include "Engine/OverlapResult.h"
#include "GAS/Ability/SkillBehavior_ChargeExpandTrace.h"
#include "P_MA/P_MA.h"

AMATargetActor_ImedDamage::AMATargetActor_ImedDamage()
{
	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root"));

	DecalComp=CreateDefaultSubobject<UDecalComponent>("Decal Component");
	DecalComp->SetupAttachment(GetRootComponent());

	if (USkillBehavior_ChargeExpandTrace* Skill = Cast<USkillBehavior_ChargeExpandTrace>(OwningAbility))
	{
		Skill->OnChargeValueChanged.AddDynamic(this, &AMATargetActor_ImedDamage::HandleChargeUpdated);
	}
}

void AMATargetActor_ImedDamage::SetTargetAreaRadius(float NewRadius)
{
	TargetAreaRadius = NewRadius;
	if (!DecalComp)
		return;

	if (TargetShape==ETraceShape::Box)
	{
		DecalComp->DecalSize = FVector(Distance/2.f, NewRadius, NewRadius);
	}
	else if (TargetShape==ETraceShape::Sphere)
	{
		DecalComp->DecalSize = FVector(1.f, NewRadius, NewRadius);
	}
}

void AMATargetActor_ImedDamage::SetTargetOptions(bool bTargetFriendly, bool bTargetEnemy)
{
	bShouldTargetFriendly = bTargetFriendly;
	bShouldTargetEnemy = bTargetEnemy;
}

void AMATargetActor_ImedDamage::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (PrimaryPC && PrimaryPC->IsLocalPlayerController())
	{
		SetActorLocation(GetTargetPoint());
	}
}

void AMATargetActor_ImedDamage::ConfirmTargetingAndContinue()
{
	FVector Center = GetActorLocation();
	FVector Forward = FVector::ZeroVector;
	if (OwningAbility && OwningAbility->GetAvatarActorFromActorInfo())
		Forward = OwningAbility->GetAvatarActorFromActorInfo()->GetActorForwardVector();

	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionShape Shape;
	if (TargetShape == ETraceShape::Sphere)
	{
		Shape.SetSphere(TargetAreaRadius);
	}
	else if (TargetShape == ETraceShape::Box)
	{
		Center = Center + Forward *(Distance/2.f);
		Shape.SetBox(FVector3f(Distance/2.f , TargetAreaRadius, TargetAreaRadius/2.f));
	}

	GetWorld()->OverlapMultiByObjectType(OverlapResults, Center, FQuat::Identity, ObjParams, Shape);
	
	TSet<AActor*> TargetActors;
	IGenericTeamAgentInterface* OwnerTeamInterface = nullptr; 
	if (OwningAbility)
	{
		OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(OwningAbility->GetAvatarActorFromActorInfo());
	}
	
	for (FOverlapResult& OverlapResult : OverlapResults)
	{
		if (OwnerTeamInterface && OwnerTeamInterface->GetTeamAttitudeTowards(*OverlapResult.GetActor()) == ETeamAttitude::Friendly && !bShouldTargetFriendly)
			continue;

		if (OwnerTeamInterface && OwnerTeamInterface->GetTeamAttitudeTowards(*OverlapResult.GetActor()) == ETeamAttitude::Hostile && !bShouldTargetEnemy)
			continue;
		
		TargetActors.Add(OverlapResult.GetActor());
	}
	
	FGameplayAbilityTargetDataHandle TargetData =UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(TargetActors.Array(), false);
	
	FGameplayAbilityTargetData_SingleTargetHit* HitLoc = new FGameplayAbilityTargetData_SingleTargetHit;
	HitLoc->HitResult.ImpactPoint = GetActorLocation();

	TargetData.Add(HitLoc);
	TargetDataReadyDelegate.Broadcast(TargetData);
}

FVector AMATargetActor_ImedDamage::GetTargetPoint() const
{
	if (!PrimaryPC || !PrimaryPC->IsLocalPlayerController())	return GetActorLocation();

	FHitResult HitResult;
	if (PrimaryPC->GetHitResultUnderCursor(ECC_Visibility, true, HitResult))
	{
		return HitResult.ImpactPoint;
	}

	return GetActorLocation();
}

void AMATargetActor_ImedDamage::HandleChargeUpdated(float NewChargeRatio)
{
	SetTargetAreaRadius(FMath::Lerp(50.f, 2000.f, NewChargeRatio));
}
