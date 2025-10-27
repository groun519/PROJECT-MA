// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Projectile/MAAbilityRangeActor.h"
#include "Components/DecalComponent.h"


// Sets default values
AMAAbilityRangeActor::AMAAbilityRangeActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root Component"));

	OutlineDecal = CreateDefaultSubobject<UDecalComponent>("Outline Decal");
	OutlineDecal->SetupAttachment(GetRootComponent());

	InnerDecal = CreateDefaultSubobject<UDecalComponent>("Inner Decal");
	InnerDecal->SetupAttachment(OutlineDecal);
}

void AMAAbilityRangeActor::SetAbilityRange(float NewRange)
{
	AbilityRange = NewRange;
	OutlineDecal->DecalSize = FVector{NewRange};
	InnerDecal	->DecalSize = FVector{NewRange};
}
