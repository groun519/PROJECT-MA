// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlatformRoot.generated.h"

class USplineComponent;
class ACore;
class UPlatformMatrixComponent;

DECLARE_MULTICAST_DELEGATE(FOnPlatformReachedEnd);

UCLASS()
class P_MA_API APlatformRoot : public AActor
{
	GENERATED_BODY()
	
public:
	APlatformRoot();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Delegate **/
	FOnPlatformReachedEnd OnPlatformReachedEnd;
	void MoveEnd();
	
	/** Matrix **/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UPlatformMatrixComponent* PlatformMatrixComponent;

	/** Core **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ACore> CoreClass;

	/** Atts Set **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeed = 1000.f;

	/** Use by Manager **/
	void SetWaitMoveIn(bool bWaitMoveIn);
	void SetHeight(bool bIsMoving);
	void SetCurSpline(USplineComponent* Spline);
	
private:
	/** Input by Manager **/
	USplineComponent* CurSpline = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root = nullptr;
	
	float Distance = 0.f;

	/** Height System **/
	float CurHeight = -100.f;
	float MovingHeight = 50.f;
	float WaitingHeight = -100.f;

	/** Core **/
	void SpawnCore();
};
