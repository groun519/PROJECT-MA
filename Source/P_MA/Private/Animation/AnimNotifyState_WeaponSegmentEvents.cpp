// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState_WeaponSegmentEvents.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Weapon/WeaponComponent.h"
#include "Abilities/GameplayAbilityTargetTypes.h"

void UAnimNotifyState_WeaponSegmentEvents::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	// 캐릭터 찾기
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner) return;
	CachedOwner = Owner; // 저장
	
	// ASC가 없다면 리턴
	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner)) return;

	// BeginEventTag 이벤트 전송 (ex. Combo01)
	if (bUseBeginEvent && BeginEventTag.IsValid())
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, BeginEventTag, FGameplayEventData());

	// 시작지점은 Tick에서 감지하지 못하기에 따로 한번 전송함.
	if (!SendCurrentLocalSegment()) return;

	// 나눈 세그먼트 포지션을 저장할 배열
	InterpMontagePos.Reset();

	// 세그먼트 개수 0 방지
	const int32 N = FMath::Max(InterpCount, 1);
	// 노티파이 길이가 거의 없는 수준이면 리턴
	if (TotalDuration <= KINDA_SMALL_NUMBER) return;

	UAnimInstance* Anim = MeshComp->GetAnimInstance();
	const UAnimMontage* Montage = Cast<UAnimMontage>(Animation);
	if (!Anim || !Montage) return;
	
	// 몽타주 타임라인 상의 NotifyState 스타트 지점
	const float StartPos = Anim->Montage_GetPosition(Montage);
	const float Step     = TotalDuration / static_cast<float>(N); // 세그먼트 간격

	// 몽타주 타임라인 상의 각 세그먼트 위치를 저장
	for (int32 i=1; i<=N; ++i)
		InterpMontagePos.Add(StartPos + Step * i); 
}

void UAnimNotifyState_WeaponSegmentEvents::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	// 잘린 구간이 없으면 리턴
	if (InterpMontagePos.Num() == 0) return;
	// 캐릭터가 없으면 리턴
	if (!CachedOwner.IsValid()) return;

	UAnimInstance* Anim = MeshComp->GetAnimInstance();
	const UAnimMontage* Montage = Cast<UAnimMontage>(Animation);
	if (!Anim || !Montage) return;

	// 현재 몽타주의 재생위치
	const float CurrPos  = Anim->Montage_GetPosition(Montage);

	while (InterpMontagePos.Num() > 0 && CurrPos >= InterpMontagePos[0])
	{
		if (!SendCurrentLocalSegment()) break; // 세그먼트마다 로컬 오프셋 전송
		InterpMontagePos.RemoveAt(0, 1, /*bAllowShrinking=*/false);
	}
}

void UAnimNotifyState_WeaponSegmentEvents::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (bUseEndEvent && EndEventTag.IsValid())
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(CachedOwner.Get(), EndEventTag, FGameplayEventData());

	InterpMontagePos.Reset();
	CachedOwner.Reset();
}

void UAnimNotifyState_WeaponSegmentEvents::SendSegment(const FVector& StartPointLocal, const FVector& EndPointLocal) const
{
	if (!CachedOwner.IsValid()) return;
	if (!AbilityEventTag.IsValid()) return; // AbilityEventTag 미할당 시 리턴

	FGameplayEventData Data;  Data.EventTag = AbilityEventTag;

	auto* LocInfo = new FGameplayAbilityTargetData_LocationInfo();
	LocInfo->SourceLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	LocInfo->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	LocInfo->SourceLocation.LiteralTransform.SetLocation(StartPointLocal); 
	LocInfo->TargetLocation.LiteralTransform.SetLocation(EndPointLocal); 

	Data.TargetData = FGameplayAbilityTargetDataHandle(LocInfo);
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(CachedOwner.Get(), AbilityEventTag, Data);
}

bool UAnimNotifyState_WeaponSegmentEvents::SendCurrentLocalSegment() const
{
	if (!CachedOwner.IsValid()) return false;
	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CachedOwner.Get())) return false;

	// 캐릭터에서 WeaponComponent를 탐색
	UWeaponComponent* WeaponComponent = CachedOwner->FindComponentByClass<UWeaponComponent>();
	if (!WeaponComponent) return false;

	const FVector BaseW = WeaponComponent->GetBladeBaseSocketLocation(); // Base 소켓의 월드 로케이션
	const FVector TipW  = WeaponComponent->GetBladeTipSocketLocation(); // Tip 소켓의 월드 로케이션
	
	const FTransform Basis = CachedOwner->GetActorTransform(); // 기준점이 될 액터의 위치 (액터 위치 기반 오프셋을 넘겨줌.)
	const FVector BaseL = Basis.InverseTransformPosition(BaseW); // 월드→로컬
	const FVector TipL  = Basis.InverseTransformPosition(TipW);

	SendSegment(BaseL, TipL); // 로컬 오프셋 전송
	return true;
}