// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SplineSector.h"
#include "Framework/MAGameMode.h"
#include "GameFramework/Actor.h"
#include "Level/PlatformRoot.h"
#include "SplineSectorManager.generated.h"

UCLASS()
class P_MA_API ASplineSectorManager : public AActor
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
public:
	ASplineSectorManager();

	UPROPERTY()
	TObjectPtr<APlatformRoot> PlatformRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<ASplineSector>> Sectors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<ASplineSector>> StartSectors;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<ASplineSector>> InBattleSectors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<ASplineSector>> OutBattleSectors;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<ASplineSector>> LoopSectors;
	
	bool IsClosePreSectorZeroVector();
	void GoBackToFirstSector();

	int32 GetNextSectorIndex(int32 CurSectorIndex);
	static ASplineSectorManager* FindSplineSectorManager(UWorld* World);

	void SetSplinesWithMAGameState(EMAGameState InMAGS);

	FORCEINLINE AMAGameMode* GetMAGameMode(){ return CachedMAGameMode; }
private:
	EMAGameState CachedPrevMAGameState = EMAGameState::Wait;
	AMAGameMode* CachedMAGameMode;
	void CachingMAGameMode();
};
