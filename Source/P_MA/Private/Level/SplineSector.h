// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "SplineSector.generated.h"


UCLASS()
class P_MA_API ASplineSector : public AActor
{
	GENERATED_BODY()

public:
	ASplineSector();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> GroundBox;

	/** Spline **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USplineComponent> Spline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SplineNum = 10;

	/** Point **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> Points;
	
protected:
	virtual void BeginPlay() override;
};
