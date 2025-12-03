#pragma once

#include "WaveData.generated.h"

USTRUCT(BlueprintType)
struct FMonsterTypes
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int32 SmallMonsterNum = 0;
	
	UPROPERTY(EditAnywhere)
	int32 MiddleMonsterNum = 0;
	
	UPROPERTY(EditAnywhere)
	int32 HeavyMonsterNum = 0;
	
	UPROPERTY(EditAnywhere)
	int32 HugeMonsterNum = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 SumTotal = 0;

	void UpdateSum()
	{
		SumTotal
			= SmallMonsterNum + MiddleMonsterNum + HeavyMonsterNum + HugeMonsterNum;
	}
};

USTRUCT(BlueprintType)
struct FWave
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FMonsterTypes NumsOfSpawnableMonster;

	UPROPERTY(EditAnywhere)
	bool bSpawnBoss = false;

	void UpdateSum()
	{
		NumsOfSpawnableMonster.UpdateSum();
	}
};

UCLASS(BlueprintType)
class UWaveData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	int32 MaxWave = 5;
	
	UPROPERTY(EditAnywhere)
	TArray<FWave> Waves;

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);
		for (FWave& Wave: Waves)	Wave.UpdateSum();
		Waves.SetNum(MaxWave);
	}
};
