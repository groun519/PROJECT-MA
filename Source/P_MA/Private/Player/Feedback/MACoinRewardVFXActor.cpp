#include "Player/Feedback/MACoinRewardVFXActor.h"

#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

static const FName NIAGARA_CoinReward_TargetLocation(TEXT("User.TargetLocation"));
static const FName NIAGARA_CoinReward_BronzeCoin(TEXT("User.BronzeCoin"));
static const FName NIAGARA_CoinReward_SilverCoin(TEXT("User.SilverCoin"));
static const FName NIAGARA_CoinReward_GoldCoin(TEXT("User.GoldCoin"));

AMACoinRewardVFXActor::AMACoinRewardVFXActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(SceneRoot);
	NiagaraComponent->SetAutoActivate(false);
}

void AMACoinRewardVFXActor::Play(const FMACoinRewardFeedbackParams& Params)
{
	check(Params.RewardVFX);
	check(Params.TargetActor);
	if (bFeedbackStarted) return;

	bFeedbackStarted = true;
	FeedbackParams = Params;
	AbsorbStartTime = GetWorld()->GetTimeSeconds() + FMath::Max(0.f, FeedbackParams.AbsorbDelay);
	SetActorLocation(FeedbackParams.SourceLocation);
	SetLifeSpan(FMath::Max(5.f, FeedbackParams.AbsorbDelay + 5.f));

	const int32 DisplayCoin = FMath::Max(0, FMath::RoundToInt(FeedbackParams.CoinAmount));
	NiagaraComponent->SetAsset(FeedbackParams.RewardVFX);
	NiagaraComponent->SetVariableFloat(NIAGARA_CoinReward_GoldCoin, static_cast<float>(DisplayCoin / 1000));
	NiagaraComponent->SetVariableFloat(NIAGARA_CoinReward_SilverCoin, static_cast<float>((DisplayCoin % 1000) / 100));
	NiagaraComponent->SetVariableFloat(NIAGARA_CoinReward_BronzeCoin, static_cast<float>((DisplayCoin % 100) / 10));
	NiagaraComponent->OnSystemFinished.AddUniqueDynamic(this, &AMACoinRewardVFXActor::HandleVFXFinished);
	NiagaraComponent->Activate(true);

	SetActorTickEnabled(true);
}

void AMACoinRewardVFXActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsValid(FeedbackParams.TargetActor))
	{
		Destroy();
		return;
	}

	if (GetWorld()->GetTimeSeconds() >= AbsorbStartTime)
	{
		NiagaraComponent->SetVariableVec3(NIAGARA_CoinReward_TargetLocation, FeedbackParams.TargetActor->GetActorLocation());
	}
}

void AMACoinRewardVFXActor::HandleVFXFinished(UNiagaraComponent* FinishedComponent)
{
	Destroy();
}
