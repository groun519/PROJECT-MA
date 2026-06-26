// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "MonstersByEnvironmentData.generated.h"

class UPCGGraph;
class AMonster;
class UTexture2D;

USTRUCT(BlueprintType)
struct FMonstersByEnvironmentData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGameplayTag EnvGameplayTag = FGameplayTag();

	UPROPERTY(EditAnywhere)
	UPCGGraph* EnvPCGGraph = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UTexture2D> DestinationIcon = nullptr;

	UPROPERTY(EditAnywhere, meta=(ClampMin = "1", ClampMax = "10000"))
	TMap<TSubclassOf<AMonster>, int32> MonsterToCoin;
};
