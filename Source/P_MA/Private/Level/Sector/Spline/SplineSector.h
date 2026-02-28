// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "Components/SplineComponent.h"
#include "PCGComponent.h"
#include "SplineSector.generated.h"

class ASplineSector;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSplineSectorUpdated, ASplineSector*);

UCLASS()
class P_MA_API ASplineSector : public AActor
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	ASplineSector();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> PCGExtentBox;

	/** Seed and Sector **/
	void SetSectorSeed(int32 InSeed = 0);
	virtual void SetRandomSeed(int32 MaxValue = INT32_MAX);
	void RegenerateWithCurrentSeed();
	FVector GetSectorBound();
	FORCEINLINE int32 GetSectorSeed() const { return SectorSeed; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	FOnSplineSectorUpdated OnSplineSectorUpdated;
	
	
	/** Spline **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USplineComponent> RoadSpline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SplineNum = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SplineOffset = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRandomAtSpawn = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> Points;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SplineWidth = 500.f;

	/** Arrow **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UArrowComponent> Arrow;

	/** PCG **/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UPCGComponent> PCGComponent;

private:
	UPROPERTY(ReplicatedUsing=OnRep_SectorSeed)
	int32 SectorSeed = 0;
	void UpdatePCGComponent();
	void UpdateSeed();

	UFUNCTION()
	void OnRep_SectorSeed();
};
