// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Projectile/MATargetActor_SelectLoc.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "Components/DecalComponent.h"



AMATargetActor_SelectLoc::AMATargetActor_SelectLoc()
{
	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root Component"));

	SkillLocDecal=CreateDefaultSubobject<UDecalComponent>("Decal Component");
	SkillLocDecal->SetupAttachment(GetRootComponent());
}
void AMATargetActor_SelectLoc::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//범위 나가도 최대 거리 끝에 고정되도록
	if (!PrimaryPC || !PrimaryPC->IsLocalPlayerController())
		return;

	const FVector TargetPoint = GetTargetPoint();
	const FVector CharacterLoc = OwningAbility->GetAvatarActorFromActorInfo()->GetActorLocation();
	const float CurrentDistance = FVector::Dist2D(TargetPoint, CharacterLoc);

	FVector FinalLoc;

	if (CurrentDistance > MaxDistance)
	{
		FVector Direction = TargetPoint - CharacterLoc;
		Direction.Z=0.f;
		Direction.Normalize();

		FVector ClampedLoc = CharacterLoc + (Direction * MaxDistance);
		ClampedLoc.Z = TargetPoint.Z;
		FinalLoc = ClampedLoc;
	}
	else
	{
		FinalLoc = TargetPoint;
	}
	SetActorLocation(FinalLoc);
	
	/* 사정거리 넘으면 데칼 색 변화
	if (PrimaryPC && PrimaryPC->IsLocalPlayerController())
		SetActorLocation(GetTargetPoint());

	if (DecalDMI && OwningAbility)
	{
		const FVector CharacterLoc = OwningAbility->GetAvatarActorFromActorInfo()->GetActorLocation();
		//캐릭터와 마우스 위치 사이 거리
		const float CurrentDistance = FVector::Dist2D(CharacterLoc, GetTargetPoint());
		//마우스가 범위 내
		if (CurrentDistance < MaxDistance)
		{
			DecalDMI->SetVectorParameterValue(FName("Color"),InRangeColor);
		}else
		{
			DecalDMI->SetVectorParameterValue(FName("Color"),OutOfRangeColor);
		}
	}
	*/
}

void AMATargetActor_SelectLoc::ConfirmTargetingAndContinue()
{
	FGameplayAbilityTargetDataHandle TargetDataHandle;
	// 타겟 히트 위치 저장 데이터
	FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit;
	NewData->HitResult.ImpactPoint = GetActorLocation();
	TargetDataHandle.Add(NewData);

	TargetDataReadyDelegate.Broadcast(TargetDataHandle);
}
FVector AMATargetActor_SelectLoc::GetTargetPoint() const
{
	if (!PrimaryPC)
		return GetActorLocation();

	FHitResult HitResult;
	if (PrimaryPC->GetHitResultUnderCursor(ECC_Visibility, true, HitResult))
	{
		return HitResult.ImpactPoint;
	}
	return GetActorLocation();
}

void AMATargetActor_SelectLoc::SetAbilityRadius(float NewRadius)
{
	AbilityRange = NewRadius;
	SkillLocDecal->DecalSize = FVector(10.f, NewRadius,NewRadius);
}
/*
void AMATargetActor_SelectLoc::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

	if (SkillLocDecal)
	{
		DecalDMI = SkillLocDecal -> CreateDynamicMaterialInstance();
	}
}
*/
