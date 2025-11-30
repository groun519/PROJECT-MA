// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "BattleSpaceSpline.generated.h"

UCLASS()
class P_MA_API ABattleSpaceSpline : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	ABattleSpaceSpline();

	/** Spline **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USplineComponent> InnerSpline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InnerSplineRadius = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRandomAtSpawn = false;
	
private:
	void UpdateInnerSpline(int32 NumPoints = 8);
};
