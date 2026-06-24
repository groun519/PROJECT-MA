// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "MonstersByEnvironmentData.generated.h"

class UPCGGraph;
class AMonster;

USTRUCT(BlueprintType)
struct FMonstersByEnvironmentData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGameplayTag EnvGameplayTag = FGameplayTag();

	UPROPERTY(EditAnywhere)
	UPCGGraph* EnvPCGGraph = nullptr;

	UPROPERTY(EditAnywhere, meta=(ClampMin = "1", ClampMax = "10000"))
	TMap<TSubclassOf<AMonster>, int32> MonsterToCoin;
};
