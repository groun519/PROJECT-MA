// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/MATargetActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/DecalComponent.h"


AMATargetActor::AMATargetActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root Component"));

	DecalComp=CreateDefaultSubobject<UDecalComponent>("Decal Component");
	DecalComp->SetupAttachment(GetRootComponent());
}
void AMATargetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PrimaryPC && PrimaryPC->IsLocalPlayerController())
		SetActorLocation(GetTargetPoint());
}

void AMATargetActor::SetTargetAreaRadius(float NewRadius)
{
	TargetAreaRadius = NewRadius;
	DecalComp->DecalSize = FVector{NewRadius};
}

void AMATargetActor::SetTargetOptions(bool bTargetFriendly, bool bTargetEnemy)
{
	bShouldTargetEnemy = bTargetEnemy;
	bShouldTargetFriendly = bTargetFriendly;
}


void AMATargetActor::ConfirmTargetingAndContinue()
{
	FGameplayAbilityTargetDataHandle TargetDataHandle;
	// 타겟 히트 위치 저장 데이터
	FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit;
	NewData->HitResult.ImpactPoint = GetActorLocation();
	TargetDataHandle.Add(NewData);

	TargetDataReadyDelegate.Broadcast(TargetDataHandle);
}

FVector AMATargetActor::GetTargetPoint() const
{
	if (!PrimaryPC)
		return GetActorLocation();

	FHitResult HitResult;
	if (PrimaryPC->GetHitResultUnderCursor(ECC_Visibility, true, HitResult))
	{
		if (bShouldDrawDebug)
		{
			DrawDebugSphere(GetWorld(), HitResult.ImpactPoint,TargetAreaRadius, 32, FColor::Red, false, 0.1f);
		}
		return HitResult.ImpactPoint;
	}
	return GetActorLocation();
}
