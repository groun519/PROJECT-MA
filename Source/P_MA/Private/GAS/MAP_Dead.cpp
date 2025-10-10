// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MAP_Dead.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/MAPlayerAttributeSet.h"
#include "Engine/OverlapResult.h"

UGAP_Dead::UGAP_Dead()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	TriggerData.TriggerTag = UMAAbilitySystemStatics::GetDeadStatTag();

	AbilityTriggers.Add(TriggerData);

	ActivationBlockedTags.RemoveTag(UMAAbilitySystemStatics::GetStunStatTag());
}

void UGAP_Dead::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (K2_HasAuthority())
	{
		AActor* Killer = TriggerEventData->ContextHandle.GetEffectCauser();
		if (!Killer || !UMAAbilitySystemStatics::IsPlayer(Killer))
		{
			Killer = nullptr;
		}

		TArray<AActor*> RewardTargets = GetRewardTargets();
		if (RewardTargets.Num() == 0 && !Killer)
		{
			K2_EndAbility();
			return;
		}

		if (Killer && !RewardTargets.Contains(Killer))
		{
			RewardTargets.Add(Killer);
		}
		float TotalGoldReward = BaseGoldReward;

		if (Killer)
		{
			float KillerGoldReward = TotalGoldReward * KillerRewardPortion;

			FGameplayEffectSpecHandle EffectSpec = MakeOutgoingGameplayEffectSpec(RewardEffect);
			
			EffectSpec.Data->SetSetByCallerMagnitude(UMAAbilitySystemStatics::GetGoldAttributeTag(), KillerGoldReward);

			K2_ApplyGameplayEffectSpecToTarget(EffectSpec, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Killer));
			
			TotalGoldReward -= KillerGoldReward;
		}
		
		float GoldPerTarget = TotalGoldReward / RewardTargets.Num();

		FGameplayEffectSpecHandle EffectSpec = MakeOutgoingGameplayEffectSpec(RewardEffect);
		EffectSpec.Data->SetSetByCallerMagnitude(UMAAbilitySystemStatics::GetGoldAttributeTag(), GoldPerTarget);

		K2_ApplyGameplayEffectSpecToTarget(EffectSpec, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(RewardTargets, true));
		K2_EndAbility();
	}
}
TArray<AActor*> UGAP_Dead::GetRewardTargets() const
{
	TSet<AActor*> OutActors;

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !GetWorld())
	{
		return OutActors.Array();
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(RewardRange);

	TArray<FOverlapResult> OverlapResults;
	if (GetWorld()->OverlapMultiByObjectType(OverlapResults, AvatarActor->GetActorLocation(), FQuat::Identity, ObjectQueryParams, CollisionShape))
	{
		for (const FOverlapResult& OverlapResult : OverlapResults)
		{
			const IGenericTeamAgentInterface* OtherTeamInterface = Cast<IGenericTeamAgentInterface>(OverlapResult.GetActor());
			if (!OtherTeamInterface || OtherTeamInterface->GetTeamAttitudeTowards(*AvatarActor) != ETeamAttitude::Hostile)
			{
				continue;
			}
			
			if (!UMAAbilitySystemStatics::IsPlayer(OverlapResult.GetActor()))
			{
				continue;
			}

			OutActors.Add(OverlapResult.GetActor());
		}
	}

	return OutActors.Array();
}