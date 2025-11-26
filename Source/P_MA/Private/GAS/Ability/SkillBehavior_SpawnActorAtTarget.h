// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "Engine/DataTable.h"
#include "SkillBehavior_SpawnActorAtTarget.generated.h"

USTRUCT(BlueprintType)
struct FElementSpawnRule : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	TSubclassOf<class AMAProjectile_GroundTargetedAOE> ProjectileClass;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	int32 ProjectileCount =1;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite,meta=(ClampMin="0.1"))
	float TravelTime = 0.5f;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	float AbilityRange = 200.f;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	float MaxDistance = 1000.f;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	float SpawnHeight = 700.f;
	
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, meta=(EditCondition="ProjectileCount > 1", EditConditionHides))
	float ProjectileSpawnDelay = 0.015f;
};
/**
 * 지점 액터 스폰
 * 플레이어가 지정한 위치에 설정한 투사체로 공격
 */
UCLASS()
class USkillBehavior_SpawnActorAtTarget : public UMASkillBehavior
{
	GENERATED_BODY()

public:
	USkillBehavior_SpawnActorAtTarget();
	
	virtual void OnActivate_Implementation() override;
	virtual void OnEndAbility_Implementation() override;
	virtual bool IsRequirePlayerInput() const override { return true; }
	virtual bool ShouldLockRotation() const override {return false;}
	virtual bool IsApplyCooldownImmediate() const override {return false;}

private:
	// 스킬 타격 범위 선택 액터
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AMATargetActor_SelectLoc> TargetActorClass;
	
	// 스킬 범위 나타낼 액터
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AMAAbilityRangeActor> RangeActorClass;
	UPROPERTY()
	TObjectPtr<class AMAAbilityRangeActor> SpawnedRangeActor;
	
	// 투사체 클래스
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDataTable> ElementSpawnRuleTable;

	float TravelTime;
	float MaxDistance;
	float AbilityRange;
	float SpawnHeight;
	int32 ProjectileCount;
	float SpawnDelay;

	TWeakObjectPtr<class UAbilityTask_WaitTargetData> WaitTargetDataTask;
	
	UFUNCTION()
	void TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data);
	UFUNCTION()
	void TargetCancelled(const FGameplayAbilityTargetDataHandle& Data);

	const FElementSpawnRule* CurrentSpawnRule = nullptr;
	FTimerHandle SpawnLoopTimer;
	FVector CachedTargetPoint;
	int32 SpawnedCount =0;

	void SpawnSingleProjectile(TSubclassOf<AMAProjectile_GroundTargetedAOE> ProjectileClass, const FVector& TargetLocation);

	UFUNCTION()
	void OnSpawnLoop();

	void CleanUp();
};
