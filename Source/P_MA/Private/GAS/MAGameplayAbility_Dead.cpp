#include "GAS/MAGameplayAbility_Dead.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAAttributeSet.h"
#include "Player/MAPlayerController.h"
#include "Player/Feedback/MACoinRewardVFXActor.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffect.h"
#include "NiagaraSystem.h"
#include "Player/MAPlayerCharacter.h"
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

}

void UMAGameplayAbility_Dead::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	AActor* DeadActor = GetAvatarActorFromActorInfo();
	if (!DeadActor || UMAAbilitySystemStatics::IsPlayer(DeadActor))
	{
		K2_EndAbility();
		return;
	}

	TArray<AActor*> RewardTargets = GetRewardTargets();
	if (RewardTargets.Num() == 0)
	{
		K2_EndAbility();
		return;
	}

	const float CoinAmount = GetAbilitySystemComponentFromActorInfo()->GetNumericAttribute(UMAAttributeSet::GetCoinAttribute());
	if (CoinAmount <= 0.f)
	{
		K2_EndAbility();
		return;
	}

	const FVector RewardSourceLocation = GetAvatarActorFromActorInfo()->GetActorLocation();
	for (AActor* RewardTarget : RewardTargets)
	{
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
	TArray<AActor*> OutActors;

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !GetWorld())
	{
		return OutActors;
	}

	const float RewardRangeSquared = FMath::Square(CoinReward.RewardRange);
	for (TActorIterator<AMAPlayerCharacter> It(GetWorld()); It; ++It)
	{
		AMAPlayerCharacter* PlayerCharacter = *It;
		if (!PlayerCharacter) continue;
		if (PlayerCharacter->IsDead()) continue;
		if (FVector::DistSquared(PlayerCharacter->GetActorLocation(), AvatarActor->GetActorLocation()) > RewardRangeSquared) continue;

		OutActors.Add(PlayerCharacter);
	}

	return OutActors;
}
