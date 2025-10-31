// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "SkillBehavior_ApplyEffectForward.generated.h"


/**
 * 전방 즉시 효과 적용
 * 캐릭터 전방 일정 거리까지 지정된 모양으로 즉시 충돌 검사
 */
UCLASS()
class USkillBehavior_ApplyEffectForward : public UMASkillBehavior
{
	GENERATED_BODY()

public:
	virtual void OnActivate_Implementation() override;
	virtual void OnEndAbility_Implementation() override;

protected:
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitDamageEventTask;

	UFUNCTION()
	void OnDamageEventReceived(FGameplayEventData Payload);
};
