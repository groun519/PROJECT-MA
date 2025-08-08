// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/WeaponComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"

UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponComponent::MeasureRangeFromMarkers()
{
	if (!BaseMarker || !TipMarker || !BladeMesh) return;

	// Blade 로컬 공간에서의 벡터를 스케일 반영하여 길이 계산 (월드로 변환해도 OK)
	const FVector BaseL = BaseMarker->GetRelativeTransform().GetLocation();
	const FVector TipL  =  TipMarker->GetRelativeTransform().GetLocation();
	const FVector LocalDelta = TipL - BaseL;

	// 비균등 스케일까지 반영
	const FVector Scale = BladeMesh->GetComponentTransform().GetScale3D();
	const FVector ScaledDelta(LocalDelta.X * Scale.X, LocalDelta.Y * Scale.Y, LocalDelta.Z * Scale.Z);

	Range = ScaledDelta.Size(); // 최종 길이를 Range에 저장
}

FVector UWeaponComponent::GetBladeForwardVector() const
{
	if (!BladeMesh) return GetComponentTransform().GetUnitAxis(EAxis::X);
	return BladeMesh->GetComponentTransform().GetUnitAxis(EAxis::X);
}

FVector UWeaponComponent::GetBladeBaseWorldLocation() const
{
	// BaseMarker 우선, 없으면 BladeMesh 중심
	if (BaseMarker) return BaseMarker->GetComponentLocation();
	if (BladeMesh)  return BladeMesh->GetComponentLocation();
	return GetComponentLocation();
}

void UWeaponComponent::CreateDefaultPartsForOwner(AActor* Owner, UWeaponComponent* Weapon, USceneComponent* Parent)
{
	if (!Owner || !Weapon) return;
	if (!Parent) Parent = Weapon;
	
	if (!Weapon->HandleMesh)
	{
		Weapon->HandleMesh = Owner->CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponHandle"));
		Weapon->HandleMesh->SetupAttachment(Parent);
		Weapon->HandleMesh->bEditableWhenInherited = true;
	}
	if (!Weapon->BladeMesh)
	{
		Weapon->BladeMesh  = Owner->CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponBlade"));
		Weapon->BladeMesh->SetupAttachment(Parent);
		Weapon->BladeMesh->bEditableWhenInherited  = true;
	}
	if (!Weapon->BaseMarker)
	{
		Weapon->BaseMarker = Owner->CreateDefaultSubobject<USceneComponent>(TEXT("WeaponBaseMarker"));
		Weapon->BaseMarker->SetupAttachment(Weapon->BladeMesh);
		Weapon->BaseMarker->bEditableWhenInherited = true;
	}
	if (!Weapon->TipMarker)
	{
		Weapon->TipMarker  = Owner->CreateDefaultSubobject<USceneComponent>(TEXT("WeaponTipMarker"));
		Weapon->TipMarker->SetupAttachment(Weapon->BladeMesh);
		Weapon->TipMarker->bEditableWhenInherited  = true;
	}
}





