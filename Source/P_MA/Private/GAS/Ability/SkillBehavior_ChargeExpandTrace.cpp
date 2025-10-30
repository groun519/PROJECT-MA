// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/SkillBehavior_ChargeExpandTrace.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "MAGameplayAbility_SkillBase.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Character/MACharacter.h"
#include "Components/DecalComponent.h"
#include "GAS/Projectile/MATargetActor_ImedDamage.h"


void USkillBehavior_ChargeExpandTrace::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();

	ChargeStartTime = GetWorld()->GetTimeSeconds();
	bIsChargeComplete = false;
	CurrentChargeRatio = 0.f;
	FinalEffectRadius = (TraceShape == ETraceShape::Sphere) ? MinTraceRadius : FixedTraceRadius;
	FinalEffectDistance = (TraceShape == ETraceShape::Box) ? MinTraceDistance : 0.f;
	FinalEffectHalfHeight = (TraceShape == ETraceShape::Box) ? FixedTraceHalfHeight : 0.f;

	//1. 인디케이터 생성
	TSubclassOf<AGameplayAbilityTargetActor> TargetClass = (TraceShape == ETraceShape::Sphere) ? SphereIndicatorClass : BoxIndicatorClass;

	WaitTargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(OwningAbility,NAME_None,EGameplayTargetingConfirmation::UserConfirmed,TargetClass);
	WaitTargetDataTask->ValidData.AddDynamic(this, &USkillBehavior_ChargeExpandTrace::TargetConfirmed);
	WaitTargetDataTask->Cancelled.AddDynamic(this, &USkillBehavior_ChargeExpandTrace::TargetCancelled);
	WaitTargetDataTask->ReadyForActivation();

	AGameplayAbilityTargetActor* TargetActor;
	WaitTargetDataTask->BeginSpawningActor(OwningAbility, TargetClass, TargetActor);
	AMATargetActor_ImedDamage* ConfirmIndicator = Cast<AMATargetActor_ImedDamage>(TargetActor);
	if (ConfirmIndicator)
	{
		ConfirmIndicator->TargetShape = TraceShape;
		ConfirmIndicator->SetTargetAreaRadius(
			(TraceShape == ETraceShape::Sphere) ? MinTraceRadius : FixedTraceRadius);
		ConfirmIndicator->SetTargetDistanceRange(MinTraceDistance);
		ConfirmIndicator->SetShouldDrawDebug(true);
		ConfirmIndicator->StartTargeting(OwningAbility);
		ChargingRangeIndicator = ConfirmIndicator;
	}
	WaitTargetDataTask->FinishSpawningActor(OwningAbility, TargetActor);
	
	//2. 차지 업데이트 타이머 시작
	GetWorld()->GetTimerManager().SetTimer(ChargeUpdateTimerHandle, this, &USkillBehavior_ChargeExpandTrace::UpdateChargeIndicator, 0.02f, true);
	
	//3. 스킬 사용 안하고 버티면 스킬 취소
	SkillTimeoutTask = UAbilityTask_WaitDelay::WaitDelay(OwningAbility, SkillTimeoutDuration);
	SkillTimeoutTask->OnFinish.AddDynamic(this, &USkillBehavior_ChargeExpandTrace::OnSkillTimeout);
	SkillTimeoutTask->ReadyForActivation();
	
	//4. 스킬 키에서 손 떼면 스킬 사용
	WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(OwningAbility);
	WaitInputReleaseTask->OnRelease.AddDynamic(this, &USkillBehavior_ChargeExpandTrace::OnChargeReleased);
	WaitInputReleaseTask->ReadyForActivation();
}

void USkillBehavior_ChargeExpandTrace::OnEndAbility_Implementation()
{
	GetWorld()->GetTimerManager().ClearTimer(ChargeUpdateTimerHandle);
	
	if (SkillTimeoutTask.IsValid())
		SkillTimeoutTask->EndTask();
	if (WaitInputReleaseTask.IsValid())
		WaitInputReleaseTask->EndTask();
	if (WaitTargetDataTask.IsValid())
		WaitTargetDataTask->EndTask();

	if (ChargingRangeIndicator)
	{
		ChargingRangeIndicator->Destroy();
		ChargingRangeIndicator = nullptr;
	}
	
	Super::OnEndAbility_Implementation();
}


void USkillBehavior_ChargeExpandTrace::UpdateChargeIndicator()
{
	if (!ChargingRangeIndicator || !IsValid(ChargingRangeIndicator))
		return;
	if (!ChargingRangeIndicator->FindComponentByClass<UDecalComponent>())
		return;
	
	const float ElapsedTime = GetWorld()->GetTimeSeconds() - ChargeStartTime;
	// 충전 비율 0~1
	CurrentChargeRatio = FMath::Clamp(ElapsedTime / MaxChargeDuration, 0.0f, 1.0f);

	if (TraceShape == ETraceShape::Sphere)
	{
		//원 모양 트레이스라면, 원 크기에 비율을 적용
		FinalEffectRadius = FMath::Lerp(MinTraceRadius, MaxTraceRadius, CurrentChargeRatio);
	}
	else if (TraceShape == ETraceShape::Box)
	{
		//박스 모양이라면, 길이에 비율을 적용
		FinalEffectDistance = FMath::Lerp(MinTraceDistance, MaxTraceDistance, CurrentChargeRatio);
	}
	OnChargeValueChanged.Broadcast(CurrentChargeRatio);
}

void USkillBehavior_ChargeExpandTrace::TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data)
{
	ExecuteConfirmedBehavior(Data);
}

void USkillBehavior_ChargeExpandTrace::TargetCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
	OwningAbility->RequestEndAbility();
}

void USkillBehavior_ChargeExpandTrace::ExecuteConfirmedBehavior(const FGameplayAbilityTargetDataHandle& Data)
{
	FGameplayAbilityActivationInfo ActivationInfo = OwningAbility->GetCurrentActivationInfo();
    if (!OwningAbility->HasAuthority(&ActivationInfo))
        return;

    UAbilitySystemComponent* ASC = OwningAbility->GetAbilitySystemComponentFromActorInfo();
    if (ASC && DamageEffect)
    {
        for (int32 i = 0; i < Data.Num(); ++i)
        {
            TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetActorsFromTargetData(Data, i);
            for (AActor* TargetActor : TargetActors)
            {
                if (!IsValid(TargetActor))
                    continue;

                UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
                if (!TargetASC)
                    continue;

                FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
                EffectContext.AddSourceObject(OwningAbility);

                FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(DamageEffect, OwningAbility->GetAbilityLevel(), EffectContext);
                if (Spec.IsValid() && Spec.Data.IsValid())
                {
                    ASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
                }
            }
        }
    }

    FVector FinalTargetLocation = FVector::ZeroVector;
    if (Data.Num() > 0 && Data.Get(0)->GetHitResult())
        FinalTargetLocation = Data.Get(0)->GetHitResult()->ImpactPoint;
    else if (Data.Num() > 0)
        FinalTargetLocation = UAbilitySystemBlueprintLibrary::GetTargetDataEndPoint(Data, 0);

    if (NiagaraEffect)
    {
        FVector EffectScale = FVector(1.f);
        if (TraceShape == ETraceShape::Sphere)
        {
            float RadiusRatio = (MaxTraceRadius > 0.f) ? (FinalEffectRadius / MaxTraceRadius) : 0.f;
            EffectScale = FVector(RadiusRatio);
        }
        else if (TraceShape == ETraceShape::Box)
        {
            EffectScale.X = (MaxTraceDistance > 0.f) ? (FinalEffectDistance / MaxTraceDistance) : 0.f;
        }

        FTransform EffectTransform(Character->GetActorRotation(), FinalTargetLocation, EffectScale);
        Character->Multicast_PlayNiagara(NiagaraEffect, EffectTransform);
    }

    OwningAbility->RequestEndAbility();
}


void USkillBehavior_ChargeExpandTrace::OnChargeReleased(float TimeHeld)
{
	if (!ChargingRangeIndicator)
		return;

	FHitResult IndicatorHit;
	IndicatorHit.Location = ChargingRangeIndicator->GetActorLocation();

	FGameplayAbilityTargetDataHandle DataHandle;
	DataHandle.Add(new FGameplayAbilityTargetData_SingleTargetHit(IndicatorHit));

	ExecuteConfirmedBehavior(DataHandle);
}

void USkillBehavior_ChargeExpandTrace::OnSkillTimeout()
{
	if (bIsChargeComplete)
		return;
	bIsChargeComplete = true;
	OwningAbility->RequestEndAbility();
}


/*
 * 직접 트레이스 그려서 데미지 주는 방식
void USkillBehavior_ChargeExpandTrace::ExecuteTraceAndApplyEffect(FVector TargetLocationOrOrigin, float EffectRadius)
{
	if (!Character || !Character->GetMesh())
		return;
	FGameplayAbilityActivationInfo ActivationInfo = OwningAbility->GetCurrentActivationInfo();
	if (!OwningAbility->HasAuthority(&ActivationInfo))
		return;

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Character);
	TArray<FHitResult> HitResults;
	bool bHit = false;
	const TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = {UEngineTypes::ConvertToObjectType(ECC_Pawn)};

	FVector EffectSpawnLocation;
	FRotator EffectRotation = Character->GetActorRotation();

	switch (TraceShape)
	{
		case ETraceShape::Sphere:
			{
				bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
					this, TargetLocationOrOrigin, TargetLocationOrOrigin,
					EffectRadius, ObjectTypes, false, IgnoreActors,
					EDrawDebugTrace::ForDuration, HitResults, true);
				EffectSpawnLocation = TargetLocationOrOrigin;
				break;
			}
		case ETraceShape::Box:
			{
				FVector ForwardVector = EffectRotation.Vector();
				FVector TraceStartLocation = Character->GetActorLocation() + ForwardVector;

				FVector HalfSize = FVector(FinalEffectDistance/2.0f, EffectRadius, FinalEffectHalfHeight);
				EffectSpawnLocation = TraceStartLocation + ForwardVector * (FinalEffectDistance/2.0f);
				bHit = UKismetSystemLibrary::BoxTraceMultiForObjects(
					this, EffectSpawnLocation, EffectSpawnLocation, HalfSize,
					EffectRotation, ObjectTypes, false, IgnoreActors,
					EDrawDebugTrace::ForDuration, HitResults, true);
				break;
			}
	}
	if (bHit && DamageEffect)
	{
		OwningAbility->ApplyDamageToHitResults(HitResults, DamageEffect);
	}
	if (NiagaraEffect)
	{
		FVector EffectScale = FVector(1.f);
		if (TraceShape == ETraceShape::Sphere)
		{
			float RadiusRatio = (MaxTraceRadius > 0.f) ? (EffectRadius / MaxTraceRadius) : 0.f;
			EffectScale = FVector(RadiusRatio);
		}
		else if (TraceShape == ETraceShape::Box)
		{
			EffectScale.X = CurrentChargeRatio;
		}
		FTransform EffectTransform(EffectRotation, EffectSpawnLocation, EffectScale);
		Character->Multicast_PlayNiagara(NiagaraEffect, EffectTransform);
	}
}
*/
