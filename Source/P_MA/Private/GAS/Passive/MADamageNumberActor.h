// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MADamageNumberActor.generated.h"

class UWidgetComponent;

UCLASS()
class AMADamageNumberActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AMADamageNumberActor();

protected:
	virtual void BeginPlay() override;

public:	
	void PlayDamageText(float DamageAmount, bool bIsCritical);

protected:
	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* DamageWidgetComp;
};
