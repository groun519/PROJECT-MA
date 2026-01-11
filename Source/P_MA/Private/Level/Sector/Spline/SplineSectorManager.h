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

	/** Platform **/
	UPROPERTY()
	TObjectPtr<APlatformRoot> PlatformRoot;

	/** Sector **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sector")
	TArray<TObjectPtr<ASplineSector>> CurSectors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sector")
	TMap<EMAGameState, FSplineSectorData> SplineSectorsByState;

	int32 GetNextSectorIndex(int32 CurSectorIndex);
	static ASplineSectorManager* FindSplineSectorManager(UWorld* World);
	void SetSplinesWithMAGameState(EMAGameState InMAGS);

	FORCEINLINE AMAGameMode* GetMAGameMode(){ return CachedMAGameMode; }
	FORCEINLINE EMAGameState GetMAGameState(){ return GetMAGameMode()->GetMAGameState(); }
	FORCEINLINE bool IsMoving(){ return bIsMoving; }
	
	/** Debug **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseStateDebug = false;
	
private:
	bool bIsMoving = false;

	/** Cache **/
	EMAGameState CachedMAGameState = EMAGameState::Wait;
	AMAGameMode* CachedMAGameMode;
	void CachingMAGameMode();

	/** Sector **/
	void GoToNextState(EMAGameState InCurState, EMAGameState InNextState);
	void SetSectorsByState(EMAGameState InState);
	FORCEINLINE bool SameAsCachedState(EMAGameState InState) { return CachedMAGameState == InState; }
};
