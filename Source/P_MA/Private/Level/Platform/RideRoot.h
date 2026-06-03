#pragma once

#include "CoreMinimal.h"
#include "RideRoot.generated.h"

class USplineComponent;
class UTextRenderComponent;
class UNiagaraComponent;
class USphereComponent;
class UDecalComponent;
class AMAPlayerCharacter;
class UPrimitiveComponent;
struct FHitResult;

DECLARE_MULTICAST_DELEGATE(FOnPlatformReachedEnd);

USTRUCT()
struct FRangeClampVisualState
{
	GENERATED_BODY()

	UPROPERTY()
	bool bVisible = false;

	UPROPERTY()
	float Size = 0.f;
};

UCLASS()
class P_MA_API ARideRoot : public AActor
{
	GENERATED_BODY()
	
public:
	ARideRoot();
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
	void SetReadyCountdownText(int32 RemainingSeconds);
	void SetRangeClampVisual(bool bVisible, float InSize);
	UPrimitiveComponent* GetRideBaseComponent() const;
	
private:
	/** Input by SplineSectorManager **/
	USplineComponent* CurSpline = nullptr;

	/** Root **/
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root = nullptr;

	/** Ready Text **/
	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* ReadyText = nullptr;
	FORCEINLINE FString FormatReadyText(int32 ReadyCount, int32 TotalCount)
	{
		return TotalCount < 0
		? FString::Printf(TEXT("%d"), ReadyCount)
		: FString::Printf(TEXT("[ %d / %d ]"), ReadyCount, TotalCount);
	};

	/** VFX **/
	UPROPERTY(VisibleAnywhere)
	UNiagaraComponent* RangeClampVFX = nullptr;

	/** Trigger **/
	UPROPERTY(VisibleAnywhere, Category = "ReadyTrigger")
	USphereComponent* MoveInTrigger = nullptr;

	/** Decal **/
	UPROPERTY(VisibleAnywhere, Category = "ReadyTrigger")
	UDecalComponent* ReadyRangeDecal = nullptr;

	UPROPERTY(ReplicatedUsing=OnRep_ReadyCounts)
	FIntPoint ReplicatedReadyCounts = FIntPoint::ZeroValue;

	UFUNCTION()
	void OnRep_ReadyCounts();

	UPROPERTY(ReplicatedUsing=OnRep_ReadyTextVisible)
	bool bReadyTextVisible = false;

	UFUNCTION()
	void OnRep_ReadyTextVisible();

	UPROPERTY(ReplicatedUsing=OnRep_RangeClampVisual)
	FRangeClampVisualState ReplicatedRangeClampVisualState;

	UFUNCTION()
	void OnRep_RangeClampVisual();

	float Distance = 0.f;
	FRangeClampVisualState AppliedRangeClampVisualState;

	void ApplyRangeClampVisual();
	void UpdateRangeClampVFXWorldLocation();
	void SyncReadyByMoveInTrigger(bool bReady);

	UFUNCTION()
	void HandleMoveInTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleMoveInTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
