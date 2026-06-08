#include "GAS/MAGameplayAbility_Dead.h"

#include "GAS/Passive/MAFloatingTextActor.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAAttributeSet.h"
#include "Player/MAPlayerController.h"
#include "Player/Feedback/MACoinRewardVFXActor.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "EngineUtils.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffect.h"
#include "NiagaraSystem.h"
#include "Player/MAPlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"

static AActor* ResolveRewardPlayerActor(AActor* Actor)
{
	if (UMAAbilitySystemStatics::IsPlayer(Actor)) return Actor;

	if (const AController* Controller = Cast<AController>(Actor))
	{
		APawn* Pawn = Controller->GetPawn();
		return UMAAbilitySystemStatics::IsPlayer(Pawn) ? Pawn : nullptr;
	}

	APawn* InstigatorPawn = Actor ? Actor->GetInstigator() : nullptr;
	return UMAAbilitySystemStatics::IsPlayer(InstigatorPawn) ? InstigatorPawn : nullptr;
}

static bool IsSelfCausedDeath(AActor* CauseActor, AActor* DeadActor)
{
	if (!CauseActor || !DeadActor) return false;
	if (CauseActor == DeadActor) return true;

	if (const AController* Controller = Cast<AController>(CauseActor))
	{
		if (Controller->GetPawn() == DeadActor) return true;
	}

	AActor* CauseRewardActor = ResolveRewardPlayerActor(CauseActor);
	AActor* DeadRewardActor = ResolveRewardPlayerActor(DeadActor);
	return CauseRewardActor && CauseRewardActor == DeadRewardActor;
}

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

	AActor* DeadActor = GetAvatarActorFromActorInfo();
	AActor* OriginalInstigator = TriggerEventData->ContextHandle.GetOriginalInstigator();
	AActor* EffectCauser = TriggerEventData->ContextHandle.GetEffectCauser();
	AActor* Killer = ResolveRewardPlayerActor(OriginalInstigator);
	if (!Killer)
	{
		Killer = ResolveRewardPlayerActor(EffectCauser);
	}

	if (!Killer && (IsSelfCausedDeath(OriginalInstigator, DeadActor) || IsSelfCausedDeath(EffectCauser, DeadActor)))
	{
		K2_EndAbility();
		return;
	}

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
	if (Killer)
	{
		const float KillerReward = RewardTargets.Num() > 0
			? TotalCoinReward * KillerRewardPortion
			: TotalCoinReward;
		CoinRewards.Add(Killer, KillerReward);
	}

	if (RewardTargets.Num() > 0)
	{
		const float SharedRewardPortion = Killer ? 1.f - KillerRewardPortion : 1.f;
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
		if (FVector::DistSquared(PlayerCharacter->GetActorLocation(), AvatarActor->GetActorLocation()) > RewardRangeSquared) continue;

		OutActors.Add(PlayerCharacter);
	}

	return OutActors;
}
