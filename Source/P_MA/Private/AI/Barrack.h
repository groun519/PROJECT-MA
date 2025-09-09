// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GenericTeamAgentInterface.h"
#include "Barrack.generated.h"

UCLASS()
class ABarrack : public AActor
{
	GENERATED_BODY()
	
public:	
	ABarrack();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere, Category = "Spawn")
	FGenericTeamId BarrackTeamId;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int MonsterPerGroup = 3;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float GroupSpawnInterval = 5.f;
	
	UPROPERTY()
	TArray<class AMonster*> MonsterPool;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	AActor* Goal;
	
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<class AMonster> MonsterClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TArray<class APlayerStart*> SpawnSpots;

	int NextSpawnSpotIndex = -1;

	const APlayerStart* GetNextSpawnSpot();

	void SpawnNewGroup();
	void SpawnNewMinions(int Amt);
	AMonster* GetNextAvaliableMonster() const;

	FTimerHandle SpawnIntervalTimerHandle;
};
