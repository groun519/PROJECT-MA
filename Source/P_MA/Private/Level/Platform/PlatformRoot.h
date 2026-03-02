// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlatformRoot.generated.h"

class USplineComponent;
class UTextRenderComponent;
class UNiagaraComponent;
class USphereComponent;
class UDecalComponent;
class AMAPlayerCharacter;
class UPrimitiveComponent;
struct FHitResult;

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

	/** Atts Set **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeed = 1000.f;

	/** Use by Manager **/
	void SetWaitMoveIn(bool bWaitMoveIn);
	void ReleaseAttachedPlayers();
	void SetCurSpline(USplineComponent* Spline);
	void SetReadyText(int32 ReadyCount, int32 TotalCount);
	void SetRangeClampVisual(bool bVisible, float InSize);
	
private:
	/** Input by Manager **/
	USplineComponent* CurSpline = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root = nullptr;

	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* ReadyText = nullptr;

	UPROPERTY(VisibleAnywhere)
	UNiagaraComponent* RangeClampVFX = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "ReadyTrigger")
	USphereComponent* MoveInTrigger = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "ReadyTrigger")
	UDecalComponent* ReadyRangeDecal = nullptr;

	UPROPERTY(EditAnywhere, Category = "ReadyTrigger", meta = (ClampMin = "0.0"))
	float MoveInTriggerRadius = 300.f;

	UPROPERTY(EditAnywhere, Category = "RangeClampVFX")
	FName RangeClampSizeParamName = TEXT("Size");
	
	UPROPERTY(ReplicatedUsing=OnRep_ReadyCounts)
	FIntPoint ReplicatedReadyCounts = FIntPoint::ZeroValue;

	UFUNCTION()
	void OnRep_ReadyCounts();

	UPROPERTY(ReplicatedUsing=OnRep_ReadyTextVisible)
	bool bReadyTextVisible = false;

	UFUNCTION()
	void OnRep_ReadyTextVisible();

	UPROPERTY(ReplicatedUsing=OnRep_RangeClampVisual)
	bool bReplicatedRangeClampVisible = false;

	UPROPERTY(ReplicatedUsing=OnRep_RangeClampVisual)
	float ReplicatedRangeClampSize = 0.f;

	UFUNCTION()
	void OnRep_RangeClampVisual();

	float Distance = 0.f;

	void ApplyRangeClampVisual();
	void UpdateRangeClampVFXWorldLocation();
	void SyncReadyByMoveInTrigger(bool bReady);

	UFUNCTION()
	void HandleMoveInTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleMoveInTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};
