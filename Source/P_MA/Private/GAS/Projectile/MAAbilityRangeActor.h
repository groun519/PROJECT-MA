// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MAAbilityRangeActor.generated.h"

UCLASS()
class AMAAbilityRangeActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AMAAbilityRangeActor();

	void SetMaxDistance(float NewRange);

private:
	UPROPERTY(VisibleAnywhere, Category="Visual")
	class UDecalComponent* RangeDecal;
};
