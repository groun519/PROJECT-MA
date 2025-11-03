// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Projectile/MATargetActor_ChargeAtTarget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/GameplayAbility.h"
#include "Components/DecalComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"

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
	float ElapsedTime = (StartTime == 0.f) ? 0.f : (GetWorld()->GetTimeSeconds() - StartTime);
	float ChargeRatio = FMath::Clamp(ElapsedTime / MaxHoldDuration, 0.f, 1.f);
/*
	FGameplayAbilityTargetDataHandle TargetDataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit();
	NewData->HitResult.ImpactPoint = GetActorLocation();
	NewData->HitResult.Distance = ChargeRatio;
	TargetDataHandle.Add(NewData);
*/
	
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
			if (Actor && Actor != OwnerActor)
			{
				TargetData->TargetActorArray.Add(Actor);
				UE_LOG(LogTemp, Warning, TEXT("Target Name = %s"), *Actor->GetName());
			}
		}
		TargetDataHandle.Add(TargetData);
	}
	FinalImpactPoint = GetActorLocation();
	FinalChargeRatio = ChargeRatio;
	
	/*
	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(CurrentSize);

	GetWorld()->OverlapMultiByObjectType(OverlapResults, GetActorLocation(), FQuat::Identity, ObjectQueryParams, CollisionShape);

	TSet<AActor*> TargetActors;
	IGenericTeamAgentInterface* OwnerTeamInterface = nullptr;
	if (OwningAbility)
	{
		OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(OwningAbility->GetAvatarActorFromActorInfo());
	}
	for (FOverlapResult& OverlapResult : OverlapResults)
	{
		if (OwnerTeamInterface && OwnerTeamInterface->GetTeamAttitudeTowards(*OverlapResult.GetActor()) ==ETeamAttitude::Friendly)
			continue;
		
		//if (OwnerTeamInterface && OwnerTeamInterface->GetTeamAttitudeTowards(*OverlapResult.GetActor()) ==ETeamAttitude::Hostile)
			//continue;
		
		TargetActors.Add(OverlapResult.GetActor());
	}
	//핸들에는 액터 배열만 담아
	FGameplayAbilityTargetDataHandle TargetDataHandle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(TargetActors.Array(), false);
	*/
	
	TargetDataReadyDelegate.Broadcast(TargetDataHandle);
}

void AMATargetActor_ChargeAtTarget::Initialize(float InMaxDistance, float InMaxSize, float InMinSize, float InMaxHoldDuration)
{
	MaxDistance = InMaxDistance;
	MaxSize = InMaxSize;
	MinSize = InMinSize;
	MaxHoldDuration = InMaxHoldDuration;

	StartTime = GetWorld()->GetTimeSeconds();
	HandleUpdate(0.f);
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
	float ChargeRatio = FMath::Clamp(InElapsedTime / MaxHoldDuration, 0.f, 1.f);
	CurrentSize = FMath::Lerp(MinSize, MaxSize, ChargeRatio);

	CollisionComp->SetSphereRadius(CurrentSize);
	SkillRangeDecal->DecalSize = FVector(10.f, CurrentSize, CurrentSize);
	SkillRangeDecal->MarkRenderStateDirty();
}
