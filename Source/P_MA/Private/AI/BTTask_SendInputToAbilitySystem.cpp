// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTTask_SendInputToAbilitySystem.h"
#include "AI/Golem/Monster.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GAS/MAAttributeSet.h"
#include "AIController.h"

EBTNodeResult::Type UBTTask_SendInputToAbilitySystem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* OwnerAIC = OwnerComp.GetAIOwner();
	if (!OwnerAIC)
		return EBTNodeResult::Failed;

	AMonster* Monster = Cast<AMonster>(OwnerAIC->GetPawn());
	if (!Monster)
		return EBTNodeResult::Failed;

	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Monster);
	if (!OwnerASC)
		return EBTNodeResult::Failed;

	// UMAAttributeSet은 ASC가 보유하므로 직접 가져옴
	const UMAAttributeSet* Attr = OwnerASC->GetSet<UMAAttributeSet>();
	if (!Attr)
		return EBTNodeResult::Failed;

	// Fury 기반 조건 분기
	const float Fury = Attr->GetFury();
	const float Threshold = Monster->FuryThreshold;

	const EMAAbilityInputID InputToUse = (Fury >= Threshold)
		? EMAAbilityInputID::Skill1
		: EMAAbilityInputID::Attack;

	OwnerASC->PressInputID(static_cast<int32>(InputToUse));

	return EBTNodeResult::Succeeded;
}
