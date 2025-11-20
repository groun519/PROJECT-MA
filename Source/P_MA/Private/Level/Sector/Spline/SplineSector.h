// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "Components/SplineComponent.h"
#include "PCGComponent.h"
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
	int32 SplineNum = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SplineSeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SplineOffset = 7.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRandomAtSpawn = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> Points;

	/** Arrow **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UArrowComponent> Arrow;

	/** PCG **/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UPCGComponent> PCGComponent;
	
protected:
	virtual void BeginPlay() override;
	void OnConstruction(const FTransform& Transform) override;

public:
	void SetRandomSeed(int MaxValue = INT32_MAX);
	FVector GetSectorBound() ;
};
