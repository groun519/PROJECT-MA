// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "SkillBehavior_AreaTarget.generated.h"

/**
 * 
 */
UCLASS()
class USkillBehavior_AreaTarget : public UMASkillBehavior
{
	GENERATED_BODY()

public:
	virtual void OnActivate_Implementation() override;
	virtual void OnEndAbility_Implementation() override;
	virtual bool IsRequirePlayerInput() const override { return true; }
protected:
	TWeakObjectPtr<class UAbilityTask_WaitTargetData> WaitTargetDataTask;
	UFUNCTION()
	void TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data);
	UFUNCTION()
	void TargetCancelled(const FGameplayAbilityTargetDataHandle& Data);
	
private:
	// 스킬 범위 선택 액터
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AMATargetActor> TargetActorClass;
	UPROPERTY(EditDefaultsOnly)
	float TargetAreaRadius = 300.f;
	UPROPERTY(EditDefaultsOnly)
	float Distance = 2000.f;

	
	// 타격 액터 생성 변수
	UPROPERTY(EditDefaultsOnly)
	FVector SpawnOffset = FVector(-800.f,0.f,0.f);
	UPROPERTY(EditDefaultsOnly)
	float SpawnHeight = 1500.f;
};
