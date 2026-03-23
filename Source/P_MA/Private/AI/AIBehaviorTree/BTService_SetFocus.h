// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_SetFocus.generated.h"

/**
 * 
 */
UCLASS()
class UBTService_SetFocus : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:	
	UBTService_SetFocus();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
