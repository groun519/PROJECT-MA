// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Passive/GAP_Launch.h"

#include "GAS/MAAbilitySystemStatics.h"

UGAP_Launch::UGAP_Launch()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	TriggerData.TriggerTag = UMAAbilitySystemStatics::GetLaunchActivateTag();

	ActivationBlockedTags.RemoveTag(FGameplayTag::RequestGameplayTag("State.Debuff.Stun"));
	AbilityTriggers.Add(TriggerData);
}

void UGAP_Launch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
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
