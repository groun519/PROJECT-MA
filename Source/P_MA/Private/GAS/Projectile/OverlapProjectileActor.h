// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Actor.h"
#include "OverlapProjectileActor.generated.h"

UCLASS()
class AOverlapProjectileActor : public AActor, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
public:	
	AOverlapProjectileActor();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Components")
	class USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Components")
	class UNiagaraComponent* NiagaraComp;


	void ShootProjectile(
		float InSpeed, float InMaxDist,
		FGenericTeamId InTeamId, FGameplayEffectSpecHandle InHitEffectHandle);

	FORCEINLINE virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override {TeamId = TeamID;}
	virtual FGenericTeamId GetGenericTeamId() const override {return TeamId;}
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void NotifyActorBeginOverlap(class AActor* Other) override;
	
protected:
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

private:
	UPROPERTY(Replicated)
	FGenericTeamId TeamId;
	
	UPROPERTY(Replicated)
	FVector MoveDir;

	UPROPERTY(Replicated)
	float ProjectileSpeed;

	UPROPERTY(EditDefaultsOnly, Category="GameplayCue")
	FGameplayTag HitGameplayCueTag;

	FGameplayEffectSpecHandle HitEffectHandle;
	FTimerHandle ShootTimerHandle;
	void TravelMaxDistanceReached();
	void SendLocalGameplayCue(AActor* CueTargetActor, const FHitResult& HitResult);
	
};
