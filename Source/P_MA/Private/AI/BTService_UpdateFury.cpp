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
	Interval = 0.2f;       // 0.2초마다 업데이트
	RandomDeviation = 0.f; // 일정한 주기로 실행
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

	// AbilitySystemComponent 가져오기
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ControlledPawn);
	if (!ASC)
		return;

	// Fury 값 읽기
	const float Fury = ASC->GetNumericAttribute(UMAAttributeSet::GetFuryAttribute());

	// Blackboard에 저장
	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		BB->SetValueAsFloat(GetSelectedBlackboardKey(), Fury);
	}

#if WITH_EDITOR
	// 디버그용 로그
	UE_LOG(LogTemp, Verbose, TEXT("[BTService_UpdateFury] %s Fury: %.2f"), *ControlledPawn->GetName(), Fury);
#endif
}
