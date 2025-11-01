// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MAProjectileBase.generated.h"

class USphereComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class UProjectileMovementComponent;
class UGameplayEffect;

UCLASS(Abstract)
class AMAProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AMAProjectileBase();

protected:
	virtual void BeginPlay() override;
	virtual void SetupCollision();

	// 충돌 감지 스피어 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USphereComponent> CollisionComponent;
	// 투사체 움직임 담당 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	// 투사체 외형 담당 나이아가라
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Components")
	UNiagaraComponent* NiagaraComponent;
	
	// 충돌 데미지 적용 게임플레이 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TSubclassOf<UGameplayEffect> DamageGameplayEffect;
	// 충돌 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	UNiagaraSystem* ImpactVFX;
	
	UPROPERTY(EditDefaultsOnly, Category="Ability")
	float LifeTime = 2.5f;

	//폭발 이펙트 재생 - 자식 클래스에서 호출
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayEffects(FVector Location);
	//폭발 광역 데미지 적용 헬퍼
	void ApplyAreaDamage(FVector OriginLocation, float DamageRadius, const FHitResult& Hit);
	//중복 폭발 방지
	bool bHasExploded = false;
};
