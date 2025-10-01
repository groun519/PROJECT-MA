#include "AI/Golem/GameplayAbility_ChargeSmash.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/MAAttributeSet.h"

bool UGameplayAbility_ChargeSmash::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                                      const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    if (const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
    {
        // Fury 값이 15 이상이어야 능력을 활성화
        float Fury = ASC->GetNumericAttribute(UMAAttributeSet::GetFuryAttribute());
        UE_LOG(LogTemp, Warning, TEXT("Fury: %f"), Fury); // Fury 로그 출력
        if (Fury < 15.f)
        {
            return false;
        }
    }

    return true;
}

void UGameplayAbility_ChargeSmash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                   const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
    {
        // Fury 값이 15 미만이면 능력을 종료
        float Fury = ASC->GetNumericAttribute(UMAAttributeSet::GetFuryAttribute());
        UE_LOG(LogTemp, Warning, TEXT("Fury: %f"), Fury); // Fury 로그 출력
        if (Fury < 15.f)
        {
            K2_EndAbility();
            return;
        }
    }

    // 능력 커밋
    if (!K2_CommitAbility()) 
    {
        UE_LOG(LogTemp, Warning, TEXT("Ability commit failed"));
        K2_EndAbility();
        return;
    }
    
    // Authority 또는 PredictionKey 확인 후 몽타주 실행
    if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
    {
        // ChargeSmash 몽타주 실행
        if (!ChargeSmashMontage)
        {
            UE_LOG(LogTemp, Warning, TEXT("ChargeSmashMontage is null"));
            K2_EndAbility();
            return;
        }

        UAbilityTask_PlayMontageAndWait* PlayChargeSmashMontageTask = 
            UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ChargeSmashMontage);
        
        // 몽타주 이벤트 처리 (몽타주 완료 시 종료)
        PlayChargeSmashMontageTask->OnBlendOut.AddDynamic(this, &UGameplayAbility_ChargeSmash::K2_EndAbility);
        PlayChargeSmashMontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_ChargeSmash::K2_EndAbility);
        PlayChargeSmashMontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_ChargeSmash::K2_EndAbility);
        PlayChargeSmashMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_ChargeSmash::K2_EndAbility);
        PlayChargeSmashMontageTask->ReadyForActivation();
    }

    // 게임플레이 이벤트 대기 (ChargeSmash 발동을 위한 이벤트 대기)
    UAbilityTask_WaitGameplayEvent* WaitLaunchEventTask = 
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetChargeSmashTag());

    // 이벤트 수신 시 발동 시작
    WaitLaunchEventTask->EventReceived.AddDynamic(this, &UGameplayAbility_ChargeSmash::StartCharging);
    WaitLaunchEventTask->ReadyForActivation();
}

FGameplayTag UGameplayAbility_ChargeSmash::GetChargeSmashTag() const
{
    return FGameplayTag::RequestGameplayTag("Ability.Monster.ChargeSmash");
}

void UGameplayAbility_ChargeSmash::StartCharging(FGameplayEventData EventData)
{
    if (K2_HasAuthority())
    {
        TArray<FHitResult> HitTargets =
            GetHitResultFromVirtualSocketTargetData(EventData.TargetData, ETeamAttitude::Hostile, /*DrawDebug*/ false, true);

        for (auto& Hit : HitTargets)
        {
            if (AActor* HitActor = Hit.GetActor())
            {
                UE_LOG(LogTemp, Warning, TEXT("ChargeSmash Hit: %s"), *HitActor->GetName());
            }
        }
    }
}
