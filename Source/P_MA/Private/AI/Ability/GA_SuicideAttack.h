// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GA_SuicideAttack.generated.h"

/**
 * 자폭 공격용 GameplayAbility
 * - 일정 거리 이내에 들어오면 자폭 몽타주 재생
 * - 몽타주 중 Damage Event 시점에 데미지 적용
 * - 몽타주가 끝난 뒤 KillDelay 만큼 기다렸다가 몬스터 풀로 반환(Deactivate)
 */
UCLASS()
class UGA_SuicideAttack : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UFUNCTION()
	void OnDamageEvent(FGameplayEventData Data);

	UFUNCTION()
	void OnDistanceCheckTick();

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnKillDelayFinished();

private:
	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* SuicideMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY()
	TArray<AActor*> IgnoreTargets;

	UPROPERTY(EditAnywhere, Category="Suicide")
	float TriggerRange = 400.f;

	UPROPERTY(EditAnywhere, Category="Suicide")
	float CheckInterval = 0.1f;

	UPROPERTY()
	bool bHasTriggeredExplosion = false;

	UPROPERTY()
	bool bKillDelayStarted = false;

	UPROPERTY(EditAnywhere, Category="Suicide")
	float KillDelay = 0.05f;
};
