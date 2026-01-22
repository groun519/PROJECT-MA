// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "MAProjectile.generated.h"

class UNiagaraComponent;
class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class AMAProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AMAProjectile();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Component")
	TObjectPtr<USphereComponent> SphereComp;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Component")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Component")
	TObjectPtr<UNiagaraComponent> Niagara;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	FGameplayEffectSpecHandle DamageEffectSpecHandle;
	
public:	
	virtual void InitializeProjectile(const FGameplayEffectSpecHandle& InSpecHandle);
	
};
