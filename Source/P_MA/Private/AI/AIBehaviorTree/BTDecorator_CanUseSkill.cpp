// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/BTDecorator_CanUseSkill.h"

#include "AI/Golem/Monster.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GAS/MAAttributeSet.h"
#include "AIController.h"

UBTDecorator_CanUseSkill::UBTDecorator_CanUseSkill()
{
	NodeName = TEXT("Can Use Skill");
}

bool UBTDecorator_CanUseSkill::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (AIC == nullptr)
	{
		return false;
	}

	AMonster* Monster = Cast<AMonster>(AIC->GetPawn());
	if (Monster == nullptr)
	{
		return false;
	}

	if (Monster->bUseFuryThreshold == false)
	{
		return false;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Monster);
	if (ASC == nullptr)
	{
		return false;
	}

	const UMAAttributeSet* Attr = ASC->GetSet<UMAAttributeSet>();
	if (Attr == nullptr)
	{
		return false;
	}

	const float Fury = Attr->GetFury();
	const float Threshold = Monster->FuryThreshold;

	return Fury >= Threshold;
}
