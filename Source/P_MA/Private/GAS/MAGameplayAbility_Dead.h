#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "MAGameplayAbility_Dead.generated.h"

class UNiagaraSystem;

USTRUCT()
struct FMACoinRewardVFXSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "VFX", meta=(DisplayName="Coin Reward VFX"))
	TObjectPtr<UNiagaraSystem> System = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "VFX", meta=(ClampMin="0.0"))
	float AbsorbDelay = 0.5f;
};

USTRUCT()
struct FMACoinRewardSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	float RewardRange = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Reward", meta=(ClampMin="0.0", ClampMax="1.0"))
	float KillerRewardPortion = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	FMACoinRewardVFXSettings VFX;
};

UCLASS()
class UMAGameplayAbility_Dead : public UMAGameplayAbility
{
	GENERATED_BODY()
public:
	UMAGameplayAbility_Dead();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
private:
	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	FMACoinRewardSettings CoinReward;

	TArray<AActor*> GetRewardTargets() const;
};
