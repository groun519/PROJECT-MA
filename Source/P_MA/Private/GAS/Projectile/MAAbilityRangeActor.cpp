// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Projectile/MAAbilityRangeActor.h"
#include "Components/DecalComponent.h"


// Sets default values
AMAAbilityRangeActor::AMAAbilityRangeActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root Component"));

	RangeDecal = CreateDefaultSubobject<UDecalComponent>("Outline Decal");
	RangeDecal->SetupAttachment(GetRootComponent());
}

void AMAAbilityRangeActor::SetMaxDistance(float NewRange)
{
	RangeDecal->DecalSize = FVector{NewRange};
}
