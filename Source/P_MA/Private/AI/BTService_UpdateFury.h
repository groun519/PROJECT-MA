// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_UpdateFury.generated.h"

/**
 * 
 */
UCLASS()
class UBTService_UpdateFury : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTService_UpdateFury();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
