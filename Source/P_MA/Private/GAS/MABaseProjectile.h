// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "MABaseProjectile.generated.h"

class USphereComponent;
class UParticleSystemComponent;
class UNiagaraSystem;
class UProjectileMovementComponent;
class UGameplayEffect;

class UNiagaraComponent;

UCLASS()
class AMABaseProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AMABaseProjectile();


	UFUNCTION(BlueprintPure, Category="Ability")
	UProjectileMovementComponent* GetProjectileMovement() const {return ProjectileMovement;}

	// 충돌 감지 스피어 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USphereComponent> CollisionComponent;
	
protected:
	virtual void BeginPlay() override;

	// 투사체 움직임 담당 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	// 투사체 외형 담당 파티클 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Components")
	UNiagaraComponent* NiagaraComponent;
	
	// 충돌 데미지 적용 게임플레이 이펙트 - 투사체 종류 결정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TSubclassOf<UGameplayEffect> DamageGameplayEffect;
	// 충돌 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	UNiagaraSystem* ImpactVFX;
	// 사운드
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TObjectPtr<USoundBase> ImpactSound;
	// 충돌 이펙트(파티클/사운드) 재생위한 게임플레이 큐 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FGameplayTag ImpactCueTag;

	//충돌 범위
	UPROPERTY(EditDefaultsOnly, Category="Ability")
	float ImpactRadius = 300.f;
	UPROPERTY(EditDefaultsOnly, Category="Ability")
	float LifeTime = 2.5f;
	
	// 충돌 시 호출 함수
	UFUNCTION()
	void OnCollisionOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayOverlapEffects();

};
