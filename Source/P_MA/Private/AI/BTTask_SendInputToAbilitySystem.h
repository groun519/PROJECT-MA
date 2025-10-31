// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "BTTask_SendInputToAbilitySystem.generated.h"

/**
 * 몬스터의 Fury 값에 따라 GAS 입력을 전송하는 BTTask
 * - Fury >= Threshold → Skill1 사용
 * - Fury < Threshold → Attack 사용
 * - Skill1 발동 시 Ability.End 이벤트를 기다림 (InProgress)
 */
UCLASS()
class UBTTask_SendInputToAbilitySystem : public UBTTaskNode
{
	GENERATED_BODY()
public:	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
