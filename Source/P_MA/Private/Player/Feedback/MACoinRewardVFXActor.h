#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MACoinRewardVFXActor.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class AMAFloatingTextActor;

USTRUCT()
struct FMACoinRewardFeedbackParams
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> RewardVFX = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY()
	TSubclassOf<AMAFloatingTextActor> FloatingTextActorClass;

	UPROPERTY()
	FVector SourceLocation = FVector::ZeroVector;

	UPROPERTY()
	float CoinAmount = 0.f;

	UPROPERTY()
	float AbsorbDelay = 0.f;
};

UCLASS()
class AMACoinRewardVFXActor : public AActor
{
	GENERATED_BODY()

public:
	AMACoinRewardVFXActor();

	void Play(const FMACoinRewardFeedbackParams& Params);

protected:
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
	void HandleVFXFinished(UNiagaraComponent* FinishedComponent);

	void SpawnFloatingText() const;

	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<UNiagaraComponent> NiagaraComponent;

	UPROPERTY(Transient)
	FMACoinRewardFeedbackParams FeedbackParams;

	float AbsorbStartTime = 0.f;
	bool bFeedbackStarted = false;
};
