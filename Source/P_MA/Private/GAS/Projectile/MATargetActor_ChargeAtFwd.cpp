// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Projectile/MATargetActor_ChargeAtFwd.h"

#include "Abilities/GameplayAbility.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "P_MA/P_MA.h"

AMATargetActor_ChargeAtFwd::AMATargetActor_ChargeAtFwd()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComp = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(RootComp);
	
	CollisionComp=CreateDefaultSubobject<UBoxComponent>("Collision Comp");
	CollisionComp->SetupAttachment(RootComp);

	SkillDecal=CreateDefaultSubobject<UDecalComponent>("Skill Decal");
	SkillDecal->SetupAttachment(RootComp);
	MaxRangeDecal=CreateDefaultSubobject<UDecalComponent>("Max Range Decal");
	MaxRangeDecal->SetupAttachment(RootComp);
	CurrentRangeDecal=CreateDefaultSubobject<UDecalComponent>("Current Range Decal");
	CurrentRangeDecal->SetupAttachment(RootComp);

	CollisionComp->SetCollisionProfileName(TEXT("Custom")); 
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Overlap); 
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComp->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Overlap);
	
	StartTime =0.f;
}


void AMATargetActor_ChargeAtFwd::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (StartTime == 0.f)
		return;

	float ElapsedTime = GetWorld()->GetTimeSeconds() - StartTime;
	HandleUpdate(ElapsedTime);
}

void AMATargetActor_ChargeAtFwd::Initialize(float InMaxDistance, float InMinDistance, float InWidth, float InDepth, float InMaxChargeDuration)
{
	MinDistance=InMinDistance;
	MaxDistance=InMaxDistance;
	SkillWidth=InWidth;
	DecalDepth=InDepth;
	MaxChargeDuration=InMaxChargeDuration;

	MaxRangeDecal->DecalSize = FVector(DecalDepth,MaxDistance,MaxDistance);

	StartTime = GetWorld()->GetTimeSeconds();
	HandleUpdate(0.f);
}

FGameplayAbilityTargetDataHandle AMATargetActor_ChargeAtFwd::GetTargetData()
{
	FGameplayAbilityTargetDataHandle TargetDataHandle;

	TArray<AActor*> TargetActors;
	CollisionComp->GetOverlappingActors(TargetActors);

	//UE_LOG(LogTemp, Warning, TEXT("[ChargeFwd] Overlapping Actors Count: %d"), TargetActors.Num());
	
	if (TargetActors.Num() > 0)
	{
		FGameplayAbilityTargetData_ActorArray* TargetData = new FGameplayAbilityTargetData_ActorArray();
		TargetData->TargetActorArray.Reserve(TargetActors.Num());

		AActor* OwnerActor = OwningAbility? OwningAbility->GetActorInfo().OwnerActor.Get() : nullptr;
		for (AActor* Actor : TargetActors)
		{
			if (Actor && Actor!=OwnerActor)
			{
				TargetData->TargetActorArray.Add(Actor);
				//UE_LOG(LogTemp, Warning, TEXT(" - Found Target: %s"), *Actor->GetName());
			}
		}
		TargetDataHandle.Add(TargetData);
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("[ChargeFwd] No actors found in collision box!"));
	}
	return TargetDataHandle;
}

void AMATargetActor_ChargeAtFwd::HandleUpdate(float InElapsedTime)
{
	float ChargeRatio = FMath::Clamp(InElapsedTime / MaxChargeDuration,0.f,1.f);
	float CurrentLength = FMath::Lerp(MinDistance, MaxDistance, ChargeRatio);

	float BoxHalfLength = CurrentLength / 2.f;
	
	CollisionComp->SetBoxExtent(FVector(BoxHalfLength, SkillWidth, 32.f));
	CollisionComp->SetRelativeLocation(FVector(BoxHalfLength, 0.f,0.f));

	SkillDecal->DecalSize = FVector(DecalDepth, SkillWidth, BoxHalfLength);
	SkillDecal->SetRelativeLocation(FVector(BoxHalfLength, 0.f, -90.f));

	CurrentRangeDecal->DecalSize = FVector(DecalDepth, CurrentLength, CurrentLength);
	CurrentRangeDecal->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	CurrentRangeDecal->MarkRenderStateDirty();
}



