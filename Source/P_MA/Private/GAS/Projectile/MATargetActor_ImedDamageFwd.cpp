// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Projectile/MATargetActor_ImedDamageFwd.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"

AMATargetActor_ImedDamageFwd::AMATargetActor_ImedDamageFwd()
{
	PrimaryActorTick.bCanEverTick = true;

	
	CollisionComp = CreateDefaultSubobject<UBoxComponent>("CollisionComponent");
	SetRootComponent(CollisionComp);

	SkillDecal = CreateDefaultSubobject<UDecalComponent>("Decal Component");
	SkillDecal->SetupAttachment(GetRootComponent());
}


void AMATargetActor_ImedDamageFwd::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMATargetActor_ImedDamageFwd::HandleChargeValueChanged(float NewChargeRatio)
{
	
	CollisionComp->SetRelativeScale3D(FVector(1.f+NewChargeRatio, 3.f, 1.f));
	SkillDecal->SetRelativeScale3D(FVector(1.f, 1.f, 1.f+NewChargeRatio));
}
