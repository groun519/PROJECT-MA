// BTService_UpdateFury.cpp
#include "AI/BTService_UpdateFury.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GAS/MAAttributeSet.h"

UBTService_UpdateFury::UBTService_UpdateFury()
{
	NodeName = TEXT("Update Fury Value");
	Interval = 0.2f;
	RandomDeviation = 0.f;
}

void UBTService_UpdateFury::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
		return;

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
		return;
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ControlledPawn);
	if (!ASC)
		return;
	
	const float Fury = ASC->GetNumericAttribute(UMAAttributeSet::GetFuryAttribute());
	
	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		BB->SetValueAsFloat(GetSelectedBlackboardKey(), Fury);
	}
}
