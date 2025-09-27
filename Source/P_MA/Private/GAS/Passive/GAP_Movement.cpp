// Fill out your copyright notice in the Description page of Project Settings.


#include "GAP_Movement.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
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

}
