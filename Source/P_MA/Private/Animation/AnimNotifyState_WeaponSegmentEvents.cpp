// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState_WeaponSegmentEvents.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Weapon/HandleComponent.h"

struct FSegmentTracker
{
	FVector Start;       // Begin 시점 블레이드 Base (World)
	FVector UnitDir;     // Begin 시점 블레이드 로컬 X(World 변환) 단위벡터
	float   TotalLength; // WeaponComponent->Range
	int32   TotalSegments; 
	int32   CurrentSegmentIndex;
	float   SegmentLength; // TotalLength / TotalSegments
};
static TMap<FTrackerKey, FSegmentTracker> GActiveTrackers;

void UAnimNotifyState_WeaponSegmentEvents::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	CachedOwner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!CachedOwner.IsValid()) return;
	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CachedOwner.Get())) return;
	
	CachedWeaponComponent = CachedOwner->FindComponentByClass<UHandleComponent>();
	if (!CachedWeaponComponent.IsValid()) return;
	
	const FVector Start   = CachedWeaponComponent->GetBladeBaseWorldLocation();
	const FVector DirUnit = CachedWeaponComponent->GetBladeForwardVector().GetSafeNormal();
	const float   Length  = FMath::Max(0.f, CachedWeaponComponent->Range);
	if (Length <= KINDA_SMALL_NUMBER || DirUnit.IsNearlyZero(KINDA_SMALL_NUMBER)) return;

	FSegmentTracker T;
	T.Start = Start;
	T.UnitDir = DirUnit;
	T.TotalLength = Length;
	T.TotalSegments = FMath::Max(InterpCount, 1); // 0 방지
	T.CurrentSegmentIndex = 0;
	T.SegmentLength = T.TotalLength / static_cast<float>(T.TotalSegments);

	const FTrackerKey Key{ MeshComp, CachedWeaponComponent, this };
	GActiveTrackers.Add(Key, T);
	TrackerKey = Key;
}

void UAnimNotifyState_WeaponSegmentEvents::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	
	FSegmentTracker* T = GActiveTrackers.Find(TrackerKey);
	if (!T) return;
	
	UHandleComponent* WC = CachedWeaponComponent.Get();
	if (!WC)
	{
		GActiveTrackers.Remove(TrackerKey);
		return;
	}
	
	const float NewRange = FMath::Max(0.f, WC->Range);
	if (!FMath::IsNearlyEqual(NewRange, T->TotalLength, 0.1f))
	{
		T->TotalLength   = NewRange;
		T->SegmentLength = T->TotalLength / static_cast<float>(T->TotalSegments);
	}

	const FVector CurrentBase = WC->GetBladeBaseWorldLocation();
	const float ProjectedDist = FVector::DotProduct(CurrentBase - T->Start, T->UnitDir);
	const float Traveled = FMath::Clamp(ProjectedDist, 0.f, T->TotalLength);

	while (T->CurrentSegmentIndex < T->TotalSegments &&
		   Traveled >= T->SegmentLength * (T->CurrentSegmentIndex + 1))
	{
		const float Apos = T->SegmentLength *  T->CurrentSegmentIndex;
		const float Bpos = T->SegmentLength * (T->CurrentSegmentIndex + 1);

		const FVector A = T->Start + T->UnitDir * Apos;
		const FVector B = T->Start + T->UnitDir * Bpos;

		AActor* Owner = CachedOwner.Get(); if (!Owner) return;
		SendSegment(Owner, A, B);

		T->CurrentSegmentIndex++;
	}
}

void UAnimNotifyState_WeaponSegmentEvents::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	GActiveTrackers.Remove(TrackerKey);
	CachedOwner.Reset();
	CachedWeaponComponent.Reset();
}

void UAnimNotifyState_WeaponSegmentEvents::SendSegment(AActor* Owner, const FVector& Start, const FVector& End) const
{
	if (!Owner) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC) return;

	FGameplayEventData Data;
	Data.EventTag = EventTag;

	auto* LocInfo = new FGameplayAbilityTargetData_LocationInfo();
	LocInfo->SourceLocation.LiteralTransform.SetLocation(Start);
	LocInfo->TargetLocation.LiteralTransform.SetLocation(End);
	Data.TargetData.Add(LocInfo);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, Data);
}
