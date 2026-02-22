// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SplineSector.h"
#include "Framework/MAGameStateTypes.h"
#include "GameFramework/Actor.h"
#include "Level/Platform//PlatformRoot.h"
#include "SplineSectorManager.generated.h"

class AMAGameMode;
class AMAPlayerCharacter;

USTRUCT()
struct FSplineSectorManagerDebugSetting
{
	GENERATED_BODY()

	// 스테이트 변경을 보고 싶을때 사용
	UPROPERTY(EditAnywhere)
	bool bUseStateDebug = false;

	// 스플라인의 마지막 위치에 도달했는지 체크하고 싶을 때 사용
	UPROPERTY(EditAnywhere)
	bool bUseSplineEndTimeDebug = false;
};

UENUM()
enum class EMoveInState : uint8
{
	Nothing		= 0,
	CanMoveIn	= 1,
	CanMoveOut	= 2,
};

USTRUCT(BlueprintType)
struct FSplineSectorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsMoving = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAutoPass = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMoveInState MoveInState = EMoveInState::Nothing;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = bIsMoving, EditConditionHides))
	TArray<TObjectPtr<ASplineSector>> Sectors;
};

USTRUCT(BlueprintType)
struct FPlayerRangeClampSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerRangeClamp")
	bool bUse = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerRangeClamp", meta = (ClampMin = "0.0"))
	float Radius = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerRangeClamp", meta = (ClampMin = "0.01"))
	float Interval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerRangeClamp", meta = (ClampMin = "0.0"))
	float DeadZone = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerRangeClamp")
	TArray<EMASectorState> States = { EMASectorState::Wait, EMASectorState::EndBattle, EMASectorState::Loop };
};

UCLASS()
class P_MA_API ASplineSectorManager : public AActor
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public:
	ASplineSectorManager();

	/** Delegate **/
	void OnHandleSectorStateChanged(EMASectorState NewState);
	UFUNCTION()
	void OnHandlePlatformReachedEnd();
	void OnHandleReadyCountChanged(int32 ReadyCount, int32 TotalCount);
	
	/** Platform **/
	UPROPERTY()
	TObjectPtr<APlatformRoot> PlatformRoot;

	/** Sector **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sector")
	TArray<TObjectPtr<ASplineSector>> CurSectors;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sector")
	TMap<EMASectorState, FSplineSectorData> SplineSectorsByState;

	int32 GetNextSectorIndex(int32 InSectorIndex);
	static ASplineSectorManager* FindSplineSectorManager(UWorld* World);

	FORCEINLINE AMAGameMode* GetMAGameMode() const { return CachedMAGameMode; }
	EMASectorState GetMASectorState() const;
	FORCEINLINE bool IsMoving(){ return bIsMoving; }
	
	/** Debug **/
	UPROPERTY(EditAnywhere)
	FSplineSectorManagerDebugSetting DebugSetting;

	/** Player Range Clamp **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerRangeClamp")
	FPlayerRangeClampSettings PlayerRangeClamp;
	
private:
	bool bIsMoving = false;
	bool bIsAutoPass = false;

	/** Cache **/
	AMAGameMode* CachedMAGameMode;
	EMASectorState CachedMASectorState = EMASectorState::Wait;
	APlatformRoot* CachedPlatformRoot;

	/** Sector **/
	// 섹터 끝에 도달했을 때, 리퀘스트 받아 사용.
	void SetSectorsByState(EMASectorState InState);
	bool IsAutoPassState(EMASectorState InState);
	void ApplyCurSplineAndSeed();
	void LogStateChange(EMASectorState InState) const;
	int32 CurSectorIndex = 0;

	/** Player Range Clamp **/
	void UpdatePlayerRangeClamp();
	void UpdatePlayerRangeClampVisual();
	bool CanApplyPlayerRangeClamp() const;
	FTimerHandle PlayerRangeClampTimerHandle;
};
