#pragma once

#include "CoreMinimal.h"
#include "GAS/Projectile/MAProjectileTypes.h"
#include "GameFramework/Actor.h"
#include "MAProjectileBase.generated.h"

class UProjectileMovementComponent;
class UDecalComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class USphereComponent;
class UAbilitySystemComponent;

UCLASS(Abstract)
class P_MA_API AMAProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AMAProjectileBase();

	void InitializeProjectile(const FMAProjectileParams& InProjectileParams);

	// Runtime speed changes must use this entry point to keep movement and homing values in sync.
	void SetProjectileSpeedMultiplier(float NewSpeedMultiplier);

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Component
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Component")
	TObjectPtr<USphereComponent> SphereComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Component")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Component")
	TObjectPtr<UDecalComponent> ProjectileDecal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Component")
	TObjectPtr<UNiagaraComponent> TrailNiagara;

	// Settings
	UPROPERTY(EditDefaultsOnly, Category="Projectile|Collision", meta=(ClampMin="0.0", UIMin="0.0"))
	float BaseRadius = 32.f;

	UPROPERTY(EditDefaultsOnly, Category="Projectile|Movement", meta=(ClampMin="0.01", UIMin="0.01"))
	float BaseSpeed = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category="Projectile|Lifetime", meta=(ClampMin="0.0", UIMin="0.0"))
	float PostHitVisualLifeSpan = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category="Projectile|Visual|Trail")
	FMAProjectileElementalVisualSettings TrailVisualSettings;

	// Visual Extension
	virtual void InitializeProjectileVisuals(const FMAProjectileParams&) {}
	virtual void OnProjectileRadiusChanged() {}
	virtual void OnProjectileElementalColorChanged() {}
	virtual void OnProjectilePendingDestroy() {}
	bool HasProjectileElementalData() const { return bRep_HasElementalVisualData; }
	const FLinearColor& GetProjectileElementalColor() const { return Rep_ElementalColor; }

private:
	// Replication
	UPROPERTY(ReplicatedUsing=ApplyProjectileRadius)
	float Rep_ProjectileRadius = 32.f;

	UPROPERTY(ReplicatedUsing=ApplyProjectileElementalColor)
	FLinearColor Rep_ElementalColor = FLinearColor::White;

	UPROPERTY(ReplicatedUsing=ApplyProjectileTrailVisuals)
	bool bRep_HasElementalVisualData = false;

	UPROPERTY(ReplicatedUsing=ApplyProjectileTrailVisuals)
	TObjectPtr<UNiagaraSystem> Rep_TrailVFX = nullptr;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBeginPendingDestroy();

	// Runtime
	FMAProjectileParams ProjectileParams;
	bool bPendingDestroy = false;
	TSet<TWeakObjectPtr<AActor>> HitActors;

	// Speed runtime
	float SpeedMultiplier = 1.f;
	float BaseHomingAccelerationMagnitude = 0.f;

	// Continuous sweep runtime
	FVector PreviousHitCheckLocation = FVector::ZeroVector;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	FHitResult BuildHitResultFromOverlap(AActor* HitActor, const FHitResult& SweepResult, UPrimitiveComponent* OtherComp) const;
	bool CanDamageActor(AActor* OtherActor) const;
	bool TryApplyHitToActor(AActor* OtherActor, const FHitResult& HitResult);
	void ApplyDamageToTarget(UAbilitySystemComponent* TargetASC, const FHitResult& HitResult);

	UFUNCTION()
	void ApplyProjectileRadius();

	UFUNCTION()
	void ApplyProjectileElementalColor();

	UFUNCTION()
	void ApplyProjectileTrailVisuals();

	void InitializeProjectileDecal();

	// Speed
	void RefreshProjectileSpeed();

	// Continuous sweep
	void CheckContinuousSweepHit();

	// Homing
	void BindHomingTarget();

	void BeginPendingDestroy();
	void ApplyPendingDestroyVisuals();
};
