// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Projectile/MATargetActor_ChargeAtTarget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "Components/DecalComponent.h"
#include "Components/SphereComponent.h"

AMATargetActor_ChargeAtTarget::AMATargetActor_ChargeAtTarget()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root Component"));

	CollisionComp = CreateDefaultSubobject<USphereComponent>("Collision Component");
	CollisionComp->SetupAttachment(GetRootComponent());

	SkillRangeDecal = CreateDefaultSubobject<UDecalComponent>("Skill Range Comp");
	SkillRangeDecal->SetupAttachment(GetRootComponent());

	StartTime = 0.f;
}

void AMATargetActor_ChargeAtTarget::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

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
	
	if (StartTime == 0.f)
		return;

	float ElapsedTime = GetWorld()->GetTimeSeconds() - StartTime;
	HandleUpdate(ElapsedTime);
}

void AMATargetActor_ChargeAtTarget::ConfirmTargetingAndContinue()
{
	FGameplayAbilityTargetDataHandle TargetDataHandle;
	
	TArray<AActor*> TargetActors;
	CollisionComp->GetOverlappingActors(TargetActors);
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
			}
		}
		//데이터 인덱스 0 : 타겟
		TargetDataHandle.Add(TargetData);
	}
	//데이터 인덱스 1 : 위치
	FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit();
	NewData->HitResult.ImpactPoint = GetActorLocation();
	NewData->HitResult.Distance = CurrentSize;
	TargetDataHandle.Add(NewData);
	
	TargetDataReadyDelegate.Broadcast(TargetDataHandle);
}

void AMATargetActor_ChargeAtTarget::Initialize(float InMaxDistance, float InMaxSize, float InMinSize, float InMaxHoldDuration)
{
	UE_LOG(LogTemp,Warning, TEXT("Charing Indicatior Initialized"));
	MaxDistance = InMaxDistance;
	MaxSize = InMaxSize;
	MinSize = InMinSize;
	MaxHoldDuration = InMaxHoldDuration;
	bIsFixedSize = false;

	StartTime = GetWorld()->GetTimeSeconds();
	HandleUpdate(0.f);
}

void AMATargetActor_ChargeAtTarget::InitializeFixed(float InMaxDistance, float InRadius)
{
	MaxDistance = InMaxDistance;
	MinSize = InRadius;
	MaxSize = InRadius;
	MaxHoldDuration = 0.f;
	bIsFixedSize=true;
	CurrentSize = InRadius;

	CollisionComp->SetSphereRadius(CurrentSize);
	SkillRangeDecal->DecalSize = FVector(10.f, CurrentSize, CurrentSize);
	SkillRangeDecal->MarkRenderStateDirty();

	StartTime = GetWorld()->GetTimeSeconds();
}

FGameplayAbilityTargetDataHandle AMATargetActor_ChargeAtTarget::GetTargetData()
{
	FGameplayAbilityTargetDataHandle TargetDataHandle;

	TArray<AActor*> TargetActors;
	CollisionComp->GetOverlappingActors(TargetActors);
	if (TargetActors.Num() > 0)
	{
		FGameplayAbilityTargetData_ActorArray* TargetData = new FGameplayAbilityTargetData_ActorArray();
		TargetData->TargetActorArray.Reserve(TargetActors.Num());

		AActor* OwnerActor = OwningAbility ? OwningAbility->GetActorInfo().OwnerActor.Get() : nullptr;
		for (AActor* Actor : TargetActors)
		{
			if (Actor && Actor!=OwnerActor)
			{
				TargetData->TargetActorArray.Add(Actor);
			}
		}
		TargetDataHandle.Add(TargetData);
	}
	FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit();
	NewData->HitResult.ImpactPoint = GetActorLocation(); // 현재 액터 위치(마우스 위치)
	NewData->HitResult.Distance = CurrentSize;           // 현재 커진 크기
	TargetDataHandle.Add(NewData);
	
	return TargetDataHandle;
}

FVector AMATargetActor_ChargeAtTarget::GetTargetPoint() const
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

void AMATargetActor_ChargeAtTarget::HandleUpdate(float InElapsedTime)
{
	if (bIsFixedSize || MaxHoldDuration <= 0.f)
	{
		CurrentSize = MinSize;
	}
	else
	{
		float ChargeRatio = FMath::Clamp(InElapsedTime / MaxHoldDuration, 0.f, 1.f);
		CurrentSize = FMath::Lerp(MinSize, MaxSize, ChargeRatio);
	}

	CollisionComp->SetSphereRadius(CurrentSize);
	SkillRangeDecal->DecalSize = FVector(10.f, CurrentSize, CurrentSize);
	SkillRangeDecal->MarkRenderStateDirty();
}
