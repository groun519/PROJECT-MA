// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlatformRoot.generated.h"

class USplineComponent;
class ACore;
class UPlatformMatrixComponent;
class UTextRenderComponent;
class UNiagaraComponent;

DECLARE_MULTICAST_DELEGATE(FOnPlatformReachedEnd);

UCLASS()
class P_MA_API APlatformRoot : public AActor
{
	GENERATED_BODY()
	
public:
	APlatformRoot();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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
	void SetReadyText(int32 ReadyCount, int32 TotalCount);
	void SetRangeClampVisual(bool bVisible, float InSize);
	ACore* GetCore() const { return CoreInstance; }
	void ResolveReadyWallOverlapsOnce();
	
private:
	/** Input by Manager **/
	USplineComponent* CurSpline = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root = nullptr;

	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* ReadyText = nullptr;

	UPROPERTY(VisibleAnywhere)
	UNiagaraComponent* RangeClampVFX = nullptr;

	UPROPERTY(EditAnywhere, Category = "RangeClampVFX")
	FName RangeClampSizeParamName = TEXT("Size");
	
	UPROPERTY(ReplicatedUsing=OnRep_ReadyCounts)
	FIntPoint ReplicatedReadyCounts = FIntPoint::ZeroValue;

	UFUNCTION()
	void OnRep_ReadyCounts();

	UPROPERTY(ReplicatedUsing=OnRep_ReadyTextVisible)
	bool bReadyTextVisible = false;

	bool bPrevWaitMoveIn = false;

	UFUNCTION()
	void OnRep_ReadyTextVisible();

	UPROPERTY(ReplicatedUsing=OnRep_RangeClampVisual)
	bool bReplicatedRangeClampVisible = false;

	UPROPERTY(ReplicatedUsing=OnRep_RangeClampVisual)
	float ReplicatedRangeClampSize = 0.f;

	UFUNCTION()
	void OnRep_RangeClampVisual();

	UPROPERTY()
	TObjectPtr<ACore> CoreInstance;
	
	float Distance = 0.f;

	/** Height System **/
	float CurHeight = -100.f;
	float MovingHeight = 50.f;
	float WaitingHeight = -100.f;

	/** Core **/
	void SpawnCore();
	void ApplyRangeClampVisual();
	void UpdateRangeClampVFXWorldLocation();
};
