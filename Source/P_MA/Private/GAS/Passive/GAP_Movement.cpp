// Fill out your copyright notice in the Description page of Project Settings.


#include "GAP_Movement.h"

#include "GameFramework/Character.h"

UGAP_Movement::UGAP_Movement()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;

    auto AddTriggerByTagName = [this](const FName& TagName)
    {
        FAbilityTriggerData TriggerData;
        TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(TagName);
        TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
        this->AbilityTriggers.Add(TriggerData);
    };
    AddTriggerByTagName(TEXT("Ability.Passive.Jump.Activate"));
    AddTriggerByTagName(TEXT("Ability.Passive.Dash.Activate"));
    AddTriggerByTagName(TEXT("Ability.Passive.Rush.Activate"));
    AddTriggerByTagName(TEXT("Ability.Passive.Teleport.Activate"));
    AddTriggerByTagName(TEXT("Ability.Passive.Launch.Activate"));
}

void UGAP_Movement::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!K2_CommitAbility())
    {
        K2_EndAbility();
        return;
    }
    ACharacter* char1 = Cast<ACharacter>(ActorInfo->AvatarActor);
	UE_LOG(LogTemp, Warning, TEXT("Player Rotation -> %s"), *char1->GetActorRotation().ToString());

    
    if (TriggerEventData->EventTag == LaunchTag)
    {
        PushSelf(TriggerEventData-> TargetData.Get(0) -> GetHitResult() -> ImpactNormal);
        K2_EndAbility();
    }
    else if (TriggerEventData->EventTag == DashTag)
    {
        PushSelf(TriggerEventData-> TargetData.Get(0) -> GetHitResult() -> ImpactNormal);
        K2_EndAbility();
    }
    else if (TriggerEventData->EventTag == JumpTag)
    {
        const FVector JumpVel = TriggerEventData -> TargetData.Get(0) -> GetHitResult() -> ImpactNormal;
        PushSelf(JumpVel);
        K2_EndAbility();
    }
    else if (TriggerEventData->EventTag == TeleportTag)
    {
		UE_LOG(LogTemp, Warning, TEXT("gap movement"));
        
        ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
        if (Character)
        {
            // 이벤트와 함께 전달된 텔레포트 목표 위치를 가져옵니다.
            const FVector TeleportLocation = TriggerEventData->TargetData.Get(0)->GetHitResult()->ImpactNormal;
            Character->TeleportTo(TeleportLocation, Character->GetActorRotation());
        }
        K2_EndAbility();
    }
}
