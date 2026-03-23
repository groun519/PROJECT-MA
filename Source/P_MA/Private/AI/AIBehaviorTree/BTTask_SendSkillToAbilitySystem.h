// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "BTTask_SendSkillToAbilitySystem.generated.h"

/**
 */
UCLASS()
class UBTTask_SendSkillToAbilitySystem : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SendSkillToAbilitySystem();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
	void HandleAbilityEnded(FGameplayTag EventTag, const FGameplayEventData* Payload);
	void RemoveAbilityEndDelegate();

private:
	UBehaviorTreeComponent* CachedOwnerComp;
	UAbilitySystemComponent* CachedASC;

	FDelegateHandle AbilityEndDelegateHandle;
	FGameplayTagContainer EndTagContainer;

	bool bWaitingForEndEvent;
};
