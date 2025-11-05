// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Projectile/MAProjectileBase.h"
#include "MAProjectile_GroundTargetedAOE.generated.h"

/**
 * 바닥 충돌형 투사체 (메테오 타입)
 */
UCLASS()
class AMAProjectile_GroundTargetedAOE : public AMAProjectileBase
{
	GENERATED_BODY()

public:
	AMAProjectile_GroundTargetedAOE();

	//투사체 떨어지는 위치, 액터 스폰 시 설정되도록 ExposeOnSpawn meta사용 => SpawnActorDeferred로 액터 생성 시작 ~ FinishSpawning 호출 전 값 설정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability", meta=(ExposeOnSpawn="true"))
	FVector TargetImpactLocation;
	//데미지 반경
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability", meta=(ExposeOnSpawn="true"))
	float DamageRadius = 300.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability", meta=(ExposeOnSpawn="true"))
	TSubclassOf<UGameplayEffect> DamageEffect;

protected:
	virtual void SetupCollision() override;
	virtual void BeginPlay() override;
private:
	UFUNCTION()
	void OnHitGround(
		UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditDefaultsOnly, Category="Ability")
	float MaxSpeed = 800.f;
	
	void Explode(const FHitResult& Hit);
	
};
