#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GameplayAbility_ChargeSmash.generated.h"

UCLASS()
class UGameplayAbility_ChargeSmash : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_ChargeSmash();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void StartCharging(FGameplayEventData EventData);

	FGameplayTag GetChargeSmashTag() const;

private:
	UFUNCTION()
	void OnMontageCompleted();

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effect")
	TSubclassOf<UGameplayEffect> DefaultDamageEffect;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	UAnimMontage* ChargeSmashMontage;
};
