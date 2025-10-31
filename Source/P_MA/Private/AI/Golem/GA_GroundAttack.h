#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GA_GroundAttack.generated.h"

UCLASS()
class UGA_GroundAttack : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_GroundAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	static FGameplayTag GetComboTargetEventTag();
	
protected:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnEndEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void HitTarget(FGameplayEventData Data);

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ChargeSmashMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> DamageEffect;

private:
	TSubclassOf<UGameplayEffect> GetDamageEffectForCurrentCombo() const;
	
	UPROPERTY()
	TArray<AActor*> IgnoreTargets;
};
