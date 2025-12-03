// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "MonstersByEnvironmentData.generated.h"

class UPCGGraph;
class AMonster;

USTRUCT(BlueprintType)
struct FEnvironmentData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	UPCGGraph* SplinePCGGraph = nullptr;

	UPROPERTY(EditAnywhere)
	UPCGGraph* BattlePCGGraph = nullptr;
};

USTRUCT(BlueprintType)
struct FBossMonsterData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<AMonster> BossMonster;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AMonster>> MinionMonsterClasses;
};

USTRUCT(BlueprintType)
struct FMonsterData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AMonster>> SmallMonsterClasses;
	
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AMonster>> MiddleMonsterClasses;
	
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AMonster>> HeavyMonsterClasses;
	
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AMonster>> HugeMonsterClasses;

	UPROPERTY(EditAnywhere)
	FBossMonsterData BossMonsterData;
};

USTRUCT(BlueprintType)
struct FMonstersByEnvironmentData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGameplayTag EnvGameplayTag = FGameplayTag();

	UPROPERTY(EditAnywhere)
	FEnvironmentData EnvironmentData = FEnvironmentData();

	UPROPERTY(EditAnywhere)
	FMonsterData MonsterData = FMonsterData();
};