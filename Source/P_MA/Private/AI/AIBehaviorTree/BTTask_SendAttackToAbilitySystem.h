// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SendAttackToAbilitySystem.generated.h"

/**
 * AI가 GAS Attack 입력만 전송하는 BTTask
 */
UCLASS()
class UBTTask_SendAttackToAbilitySystem : public UBTTaskNode
{
	GENERATED_BODY()
public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
