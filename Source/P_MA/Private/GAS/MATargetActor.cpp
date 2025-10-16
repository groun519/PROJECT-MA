// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/MATargetActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "Components/DecalComponent.h"



AMATargetActor::AMATargetActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root Component"));

	SkillLocDecal=CreateDefaultSubobject<UDecalComponent>("Decal Component");
	SkillLocDecal->SetupAttachment(GetRootComponent());
}
void AMATargetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PrimaryPC && PrimaryPC->IsLocalPlayerController())
		SetActorLocation(GetTargetPoint());

	if (DecalDMI && OwningAbility)
	{
		const FVector CharacterLoc = OwningAbility->GetAvatarActorFromActorInfo()->GetActorLocation();
		//캐릭터와 마우스 위치 사이 거리
		const float CurrentDistance = FVector::Dist2D(CharacterLoc, GetTargetPoint());
		//마우스가 범위 내
		if (CurrentDistance < Distance)
		{
			DecalDMI->SetVectorParameterValue(FName("Color"),InRangeColor);
		}else
		{
			DecalDMI->SetVectorParameterValue(FName("Color"),OutOfRangeColor);
		}
	}
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

void AMATargetActor::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

	if (SkillLocDecal)
	{
		DecalDMI = SkillLocDecal -> CreateDynamicMaterialInstance();
	}
}

FVector AMATargetActor::GetTargetPoint() const
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

void AMATargetActor::SetTargetAreaRadius(float NewRadius)
{
	TargetAreaRadius = NewRadius;
	SkillLocDecal->DecalSize = FVector{NewRadius};
}
