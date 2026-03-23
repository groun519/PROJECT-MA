// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTS_SetFocus.generated.h"

/**
 * 
 */
UCLASS()
class UBTS_SetFocus : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:	
	UBTS_SetFocus();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
