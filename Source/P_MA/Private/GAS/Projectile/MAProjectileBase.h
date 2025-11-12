// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Actor.h"
#include "MAProjectileBase.generated.h"


class UGameplayEffect;

UCLASS()
class AMAProjectileBase : public AActor, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
public:	
	AMAProjectileBase();

	virtual void ShootProjectile(
		float InSpeed, float InMaxDist, float InExplodeRange,
		FGenericTeamId InTeamId, FGameplayEffectSpecHandle InHitEffectHandle);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class USphereComponent* CollisionComp;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Components")
	class UNiagaraComponent* NiagaraComp;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TSubclassOf<UGameplayEffect> AdditionalEffect;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual FGenericTeamId GetGenericTeamId() const override {return TeamId;}
	FORCEINLINE virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override {TeamId = TeamID;}

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	//폭발 광역 데미지 적용 헬퍼
	void ApplyAreaDamage(FVector OriginLocation, float DamageRadius, const FHitResult& Hit);
	//중복 폭발 방지
	bool bHasExploded = false;
	
	UPROPERTY(Replicated)
	FGenericTeamId TeamId;

	UPROPERTY(Replicated)
	FVector MoveDir;

	UPROPERTY(Replicated)
	float ProjectileSpeed;

	UPROPERTY(Replicated)
	float ExplodeRadius;

	UPROPERTY(EditDefaultsOnly, Category="Cue Tag")
	FGameplayTag HitGameplayCueTag;

	FGameplayEffectSpecHandle HitEffectHandle;
	FTimerHandle ShootTimerHandle;
	void SendLocalGameplayCue(AActor* CueTargetActor, const FHitResult& HitResult);
};
