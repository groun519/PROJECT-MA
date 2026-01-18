// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SplineSector.h"
#include "Framework/MAGameMode.h"
#include "GameFramework/Actor.h"
#include "Level/Platform//PlatformRoot.h"
#include "SplineSectorManager.generated.h"

USTRUCT(BlueprintType)
struct FSplineSectorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsMoving = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAutoPass = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = bIsMoving, EditConditionHides))
	TArray<TObjectPtr<ASplineSector>> Sectors;
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
	void OnHandleGameStateChanged(EMAGameState NewState);
	UFUNCTION()
	void OnHandlePlatformReachedEnd();
	void OnHandleAllPlayersReady();
	
	/** Platform **/
	UPROPERTY()
	TObjectPtr<APlatformRoot> PlatformRoot;

	/** Sector **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sector")
	TArray<TObjectPtr<ASplineSector>> CurSectors;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sector")
	TMap<EMAGameState, FSplineSectorData> SplineSectorsByState;

	int32 GetNextSectorIndex(int32 InSectorIndex);
	static ASplineSectorManager* FindSplineSectorManager(UWorld* World);

	FORCEINLINE AMAGameMode* GetMAGameMode(){ return CachedMAGameMode; }
	FORCEINLINE EMAGameState GetMAGameState(){ return GetMAGameMode()->GetMAGameState(); }
	FORCEINLINE bool IsMoving(){ return bIsMoving; }
	
	/** Debug **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseStateDebug = false;
	
private:
	bool bIsMoving = false;



	/** Cache **/
	AMAGameMode* CachedMAGameMode;
	EMAGameState CachedMAGameState = EMAGameState::Wait;
	APlatformRoot* CachedPlatformRoot;

	/** Sector **/
	// 섹터 끝에 도달했을 때, 리퀘스트 받아 사용.
	void GoToNextState(EMAGameState InNextState);
	void SetSectorsByState(EMAGameState InState);
	bool HandleRepeatState(EMAGameState InState);
	void ApplySplineSelection();
	void LogStateChange(EMAGameState InState) const;
	int32 CurSectorIndex = 0;
};
