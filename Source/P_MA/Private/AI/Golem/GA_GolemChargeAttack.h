#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_GolemChargeAttack.generated.h"

UCLASS()
class UGA_GolemChargeAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_GolemChargeAttack();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	/** 발동 최소 거리 (이상일 때만 발동) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attack")
	float MinDistance = 600.f;

	/** 차징 공격 애니메이션 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attack")
	UAnimMontage* ChargeMontage = nullptr;

	/** 디버그 로그 출력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Debug")
	bool bDebug = false;

private:
	UFUNCTION()
	void OnMontageFinished();
};
