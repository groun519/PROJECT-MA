#include "GAS/MAGameplayAbility_Dead.h"

#include "GAS/Passive/MAFloatingTextActor.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAAttributeSet.h"
#include "Player/MAPlayerController.h"
#include "Player/Feedback/MACoinRewardVFXActor.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffect.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"

UMAGameplayAbility_Dead::UMAGameplayAbility_Dead()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	TriggerData.TriggerTag = UMAAbilitySystemStatics::GetDeadStatTag();

	AbilityTriggers.Add(TriggerData);

	ActivationBlockedTags.RemoveTag(UMAAbilitySystemStatics::GetStunStatTag());

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> CoinRewardVFXFinder(TEXT("/Game/_WorkSpace/AI/Monsters/CoinDrop/NS_CoinDrop.NS_CoinDrop"));
	check(CoinRewardVFXFinder.Succeeded());
	CoinReward.VFX.System = CoinRewardVFXFinder.Object;

	static ConstructorHelpers::FClassFinder<AMAFloatingTextActor> FloatingTextActorFinder(TEXT("/Game/_WorkSpace/GameplayAbilities/Needs/BP_DamageNumber"));
	check(FloatingTextActorFinder.Succeeded());
	CoinReward.FloatingTextActorClass = FloatingTextActorFinder.Class;
}

void UMAGameplayAbility_Dead::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	check(TriggerEventData);

	AActor* Killer = TriggerEventData->ContextHandle.GetEffectCauser();
	if (!Killer || !UMAAbilitySystemStatics::IsPlayer(Killer)) Killer = nullptr;

	TArray<AActor*> RewardTargets = GetRewardTargets();
	RewardTargets.Remove(Killer);
	if (RewardTargets.Num() == 0 && !Killer)
	{
		K2_EndAbility();
		return;
	}

	const float TotalCoinReward = GetAbilitySystemComponentFromActorInfo()->GetNumericAttribute(UMAAttributeSet::GetCoinAttribute());
	if (TotalCoinReward <= 0.f)
	{
		K2_EndAbility();
		return;
	}

	TMap<AActor*, float> CoinRewards;
	const float KillerRewardPortion = FMath::Clamp(CoinReward.KillerRewardPortion, 0.f, 1.f);
	const float SharedRewardPortion = 1.f - KillerRewardPortion;
	if (Killer)
	{
		const float KillerReward = RewardTargets.Num() > 0
			? TotalCoinReward * KillerRewardPortion
			: TotalCoinReward;
		CoinRewards.Add(Killer, KillerReward);
	}

	if (RewardTargets.Num() > 0)
	{
		const float CoinPerTarget = TotalCoinReward * SharedRewardPortion / RewardTargets.Num();
		for (AActor* RewardTarget : RewardTargets)
		{
			CoinRewards.Add(RewardTarget, CoinPerTarget);
		}
	}

	const FVector RewardSourceLocation = GetAvatarActorFromActorInfo()->GetActorLocation();
	for (const TPair<AActor*, float>& RewardEntry : CoinRewards)
	{
		AActor* RewardTarget = RewardEntry.Key;
		const float CoinAmount = RewardEntry.Value;
		if (!RewardTarget || CoinAmount <= 0.f) continue;

		if (UAbilitySystemComponent* RewardASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(RewardTarget))
		{
			RewardASC->ApplyModToAttribute(UMAAttributeSet::GetCoinAttribute(), EGameplayModOp::Additive, CoinAmount);
		}

		if (const APawn* RewardPawn = Cast<APawn>(RewardTarget))
		{
			if (AMAPlayerController* RewardPC = Cast<AMAPlayerController>(RewardPawn->GetController()); RewardPC && this->CoinReward.VFX.System)
			{
				FMACoinRewardFeedbackParams FeedbackParams;
				FeedbackParams.RewardVFX = this->CoinReward.VFX.System;
				FeedbackParams.TargetActor = RewardTarget;
				FeedbackParams.FloatingTextActorClass = this->CoinReward.FloatingTextActorClass;
				FeedbackParams.SourceLocation = RewardSourceLocation;
				FeedbackParams.CoinAmount = CoinAmount;
				FeedbackParams.AbsorbDelay = this->CoinReward.VFX.AbsorbDelay;
				RewardPC->ClientPlayCoinRewardFeedback(FeedbackParams);
			}
		}
	}

	K2_EndAbility();
}

TArray<AActor*> UMAGameplayAbility_Dead::GetRewardTargets() const
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
	CollisionShape.SetSphere(CoinReward.RewardRange);

	TArray<FOverlapResult> OverlapResults;
	if (GetWorld()->OverlapMultiByObjectType(OverlapResults, AvatarActor->GetActorLocation(), FQuat::Identity, ObjectQueryParams, CollisionShape))
	{
		for (const FOverlapResult& OverlapResult : OverlapResults)
		{
			const IGenericTeamAgentInterface* OtherTeamInterface = Cast<IGenericTeamAgentInterface>(OverlapResult.GetActor());
			if (!OtherTeamInterface || OtherTeamInterface->GetTeamAttitudeTowards(*AvatarActor) != ETeamAttitude::Hostile)
				continue;
			
			if (!UMAAbilitySystemStatics::IsPlayer(OverlapResult.GetActor()))
				continue;

			OutActors.Add(OverlapResult.GetActor());
		}
	}

	return OutActors.Array();
}
