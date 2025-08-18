// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"

UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FVector UWeaponComponent::GetBladeBaseSocketLocation()
{
	return GetSocketLocation(BaseSocketName);
}

FVector UWeaponComponent::GetBladeTipSocketLocation()
{
	return GetSocketLocation(TipSocketName);
}


