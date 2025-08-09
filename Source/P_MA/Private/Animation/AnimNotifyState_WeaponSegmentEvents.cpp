// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState_WeaponSegmentEvents.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Weapon/BladeComponent.h"
#include "Abilities/GameplayAbilityTargetTypes.h"

struct FSegmentTracker
{
	FVector Base;       // Begin 시점 블레이드 Base (World)
	FVector UnitDir;
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
	
	if (bUseBeginEvent &&
	BeginEventTag.IsValid())
	{
		AActor* Owner = CachedOwner.IsValid() ? CachedOwner.Get() : (MeshComp ? MeshComp->GetOwner() : nullptr);
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, BeginEventTag, FGameplayEventData());
	}

	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CachedOwner.Get())) return;

	CachedBladeComponent = CachedOwner->FindComponentByClass<UBladeComponent>();
	if (!CachedBladeComponent.IsValid()) return;
	
	if (!CachedBladeComponent->DoesSocketExist(CachedBladeComponent->BaseSocketName) ||
		!CachedBladeComponent->DoesSocketExist(CachedBladeComponent->TipSocketName)) return;
	
	const FVector Base   = CachedBladeComponent->GetBladeBaseSocketLocation();
	const FVector Tip	  = CachedBladeComponent->GetBladeTipSocketLocation();
	
	const FVector Dir = (Tip - Base).GetSafeNormal();
	const float   Len  = FMath::Max(0.f, CachedBladeComponent->Range);
	
	if (Len <= KINDA_SMALL_NUMBER || Dir.IsNearlyZero(KINDA_SMALL_NUMBER)) return;

	FSegmentTracker T;
	T.Base = Base;
	T.UnitDir = Dir;
	T.TotalLength = Len;
	T.TotalSegments = FMath::Max(InterpCount, 1); // 0 방지
	T.CurrentSegmentIndex = 0;
	T.SegmentLength = T.TotalLength / static_cast<float>(T.TotalSegments);

	const FTrackerKey Key{ MeshComp, CachedBladeComponent, this };
	GActiveTrackers.Add(Key, T);
	TrackerKey = Key;
}

void UAnimNotifyState_WeaponSegmentEvents::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	
	FSegmentTracker* T = GActiveTrackers.Find(TrackerKey);
	if (!T) return;
	
	UBladeComponent* BladeComp = CachedBladeComponent.Get();
	if (!BladeComp) { GActiveTrackers.Remove(TrackerKey); return; }

	const float NewRangeLen = FVector::Distance(
	BladeComp->GetBladeBaseSocketLocation(),
	BladeComp->GetBladeTipSocketLocation()
);
	if (!FMath::IsNearlyEqual(NewRangeLen, T->TotalLength, 0.1f)) {
		T->TotalLength   = NewRangeLen;
		T->SegmentLength = T->TotalLength / float(T->TotalSegments);
	}

	const FVector CurrentBase = BladeComp->GetBladeBaseSocketLocation();
	const float ProjectedDist = FVector::DotProduct(CurrentBase - T->Base, T->UnitDir);
	const float Traveled = FMath::Clamp(ProjectedDist, 0.f, T->TotalLength);

	while (T->CurrentSegmentIndex < T->TotalSegments &&
		   Traveled >= T->SegmentLength * (T->CurrentSegmentIndex + 1))
	{
		const float Apos = T->SegmentLength *  T->CurrentSegmentIndex;
		const float Bpos = T->SegmentLength * (T->CurrentSegmentIndex + 1);

		const FVector A = T->Base + T->UnitDir * Apos;
		const FVector B = T->Base + T->UnitDir * Bpos;

		if (AActor* Owner = CachedOwner.Get()) {
			SendSegment(Owner, A, B);
		} else {
			GActiveTrackers.Remove(TrackerKey);
			return;
		}
		T->CurrentSegmentIndex++;
	}
}

void UAnimNotifyState_WeaponSegmentEvents::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (bUseEndEvent &&
		EndEventTag.IsValid())
	{
		AActor* Owner = CachedOwner.IsValid() ? CachedOwner.Get() : (MeshComp ? MeshComp->GetOwner() : nullptr);
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EndEventTag, FGameplayEventData());
	}
	
	GActiveTrackers.Remove(TrackerKey);
	CachedOwner.Reset();
	CachedBladeComponent.Reset();
}

void UAnimNotifyState_WeaponSegmentEvents::SendSegment(AActor* Owner, const FVector& Start, const FVector& End) const
{
	if (!Owner) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC) return;

	FGameplayEventData Data;
	Data.EventTag = AbilityEventTag;

	auto* LocInfo = new FGameplayAbilityTargetData_LocationInfo();
	LocInfo->SourceLocation.LiteralTransform.SetLocation(Start);
	LocInfo->TargetLocation.LiteralTransform.SetLocation(End);
	Data.TargetData.Add(LocInfo);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, AbilityEventTag, Data);
}
