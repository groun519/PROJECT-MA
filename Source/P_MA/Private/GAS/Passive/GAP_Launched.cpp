// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Passive/GAP_Launched.h"

#include "GAS/MAGameplayAbility.h"


UGAP_Launched::UGAP_Launched()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;		//서버에서만 실행되도록 -> 서버가 트리거하기 때문
	// How this ability can be triggered by an event
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;	//Gameplay Event 기반의 능력 트리거 하나 있다
	TriggerData.TriggerTag = GetLaunchedAbilityActivationTag();					//그 태그도 여기 있다

	AbilityTriggers.Add(TriggerData);
}

void UGAP_Launched::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (K2_HasAuthority())
	{
		PushSelf(TriggerEventData-> TargetData.Get(0) -> GetHitResult() -> ImpactNormal);
		K2_EndAbility();
	}
}

FGameplayTag UGAP_Launched::GetLaunchedAbilityActivationTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Passive.Launch.Activate");
}
