#pragma once

#include "CoreMinimal.h"
#include "Framework/MAGameStateTypes.h"
#include "GameFramework/Actor.h"
#include "StageManager.generated.h"

class AMAGameMode;

DECLARE_MULTICAST_DELEGATE(FOnStageChangeEnvRequested);

UENUM(BlueprintType)
enum class EStageType : uint8
{
	Normal = 0,
	Boss = 1,
};

USTRUCT(BlueprintType)
struct FStageSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage")
	EStageType StageType = EStageType::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage")
	bool bSpawnTrader = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage")
	bool bChangeEnv = false;
};

UCLASS()
class P_MA_API AStageManager : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	AStageManager();

	FOnStageChangeEnvRequested OnStageChangeEnvRequested;
	
	/** Helper **/
	UFUNCTION(BlueprintPure, Category="Stage")
	int32 GetCurrentStageIndex() const { return CurStageCycleData.Stage; }

	UFUNCTION(BlueprintPure, Category="Stage")
	int32 GetCurrentCycleIndex() const { return CurStageCycleData.Round; }

	UFUNCTION(BlueprintPure, Category="Stage")
	const FStageSetting& GetCurrentStageSetting() const;

	/** Stage Setting **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stage")
	TArray<FStageSetting> StageSettings;

private:
	void OnHandleSectorStateChanged(EMASectorState NewState);
	void AdvanceStage();
	// 최대 스테이지 수 getter
	FORCEINLINE int32 GetMaxStageCount() { return StageSettings.Num(); }
	
	/** Cache **/
	UPROPERTY()
	AMAGameMode* CachedMAGameMode = nullptr;
	EMASectorState CachedMASectorState = EMASectorState::Wait;
	/**/

	// 스테이지 정보 관리
	UPROPERTY(VisibleAnywhere, Category="Stage")
	FStageCycle CurStageCycleData;
};
