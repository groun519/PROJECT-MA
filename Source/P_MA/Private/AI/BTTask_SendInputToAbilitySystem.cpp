// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_SendInputToAbilitySystem.h"
#include "AI/Golem/Monster.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/MASkillSystemTypes.h"
#include "AIController.h"

UBTTask_SendInputToAbilitySystem::UBTTask_SendInputToAbilitySystem()
{
	NodeName = TEXT("Use Selected Monster Skill");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_SendInputToAbilitySystem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC)
		return EBTNodeResult::Failed;

	AMonster* Monster = Cast<AMonster>(AIC->GetPawn());
	if (!Monster)
		return EBTNodeResult::Failed;

	UMASkillManagerComponent* SkillManager = Monster->GetSkillManagerComponent();
	UAbilitySystemComponent* AbilitySystemComponent = Monster->GetAbilitySystemComponent();

	if (!Monster->HasSelectedSkill())
		return EBTNodeResult::Failed;

	const FGameplayTag SlotTagToUse = Monster->GetSelectedSkillSlotTag();
	const int32 SlotInputID = FMASkillSystemStatics::ResolveSlotInputID(SlotTagToUse);
	if (SlotInputID == INDEX_NONE) return EBTNodeResult::Failed;

	AIC->StopMovement();
	AbilitySystemComponent->AbilityLocalInputPressed(SlotInputID);
	const bool bActivated = SkillManager->TryActivateSkill(SlotTagToUse);
	AbilitySystemComponent->AbilityLocalInputReleased(SlotInputID);
	if (!bActivated) return EBTNodeResult::Failed;

	UMASkillAbility* SkillAbility = SkillManager->GetSkillAbility(SlotTagToUse);
	if (!SkillAbility || !SkillAbility->IsActive()) return EBTNodeResult::Succeeded;

	OwnerBehaviorTree = &OwnerComp;
	ActiveSkillAbility = SkillAbility;
	SkillDeactivatedHandle = SkillAbility->OnSkillDeactivated().AddUObject(this, &UBTTask_SendInputToAbilitySystem::HandleSkillDeactivated);
	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_SendInputToAbilitySystem::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UnbindSkillDeactivated();
	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_SendInputToAbilitySystem::HandleSkillDeactivated()
{
	UBehaviorTreeComponent* BehaviorTree = OwnerBehaviorTree.Get();
	UnbindSkillDeactivated();
	if (BehaviorTree) FinishLatentTask(*BehaviorTree, EBTNodeResult::Succeeded);
}

void UBTTask_SendInputToAbilitySystem::UnbindSkillDeactivated()
{
	if (UMASkillAbility* SkillAbility = ActiveSkillAbility.Get())
	{
		SkillAbility->OnSkillDeactivated().Remove(SkillDeactivatedHandle);
	}
	OwnerBehaviorTree.Reset();
	ActiveSkillAbility.Reset();
	SkillDeactivatedHandle.Reset();
}
