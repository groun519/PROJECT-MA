// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "SkillBehavior_SpawnActorAtTarget.generated.h"


USTRUCT(BlueprintType)
struct FElementSpawnRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	TSubclassOf<class AMAProjectile_GroundTargetedAOE> ProjectileClass;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	int32 ProjectileCount =1;
	
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, meta=(EditCondition="ProjectileCount > 1", EditConditionHides))
	float ProjectileSpawnDelay = 0.f;
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
	FElementSpawnRule DefaultProjectile;
	UPROPERTY(EditDefaultsOnly)
	TMap<FName, FElementSpawnRule> OverrideProjectiles;
	
	// 투사체 속도
	UPROPERTY(EditDefaultsOnly)
	float ProjectileSpeed = 700.f;
	// 스킬 사이즈 (타격 범위)
	UPROPERTY(EditDefaultsOnly)
	float AbilityRange = 300.f;
	// 스킬 사거리
	UPROPERTY(EditDefaultsOnly)
	float MaxDistance = 2000.f;
	
	// 타격 액터 생성 변수
	UPROPERTY(EditDefaultsOnly)
	float SpawnHeight = 1500.f;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> ShortCooldownEffect;

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
