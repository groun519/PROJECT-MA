#pragma once

#include "CoreMinimal.h"
#include "GAS/Projectile/MAProjectileTypes.h"
#include "GameFramework/Actor.h"
#include "MAProjectile.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UProjectileMovementComponent;
class USphereComponent;
class UAbilitySystemComponent;
class UMASkillModuleInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProjectileHitSignature, AActor*, HitActor);

UCLASS()
class AMAProjectile : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
public:	
	AMAProjectile();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Component")
	TObjectPtr<USphereComponent> SphereComp;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Component")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Component")
	TObjectPtr<UNiagaraComponent> Niagara;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Component")
	TObjectPtr<UNiagaraComponent> TrailNiagara;

	UPROPERTY(EditDefaultsOnly, Category="Projectile|Visual|Main")
	FMAProjectileElementalVisualSettings MainVisualSettings;

	UPROPERTY(EditDefaultsOnly, Category="Projectile|Visual|Trail")
	FMAProjectileElementalVisualSettings TrailVisualSettings;

	UPROPERTY(EditDefaultsOnly, Category="Projectile|Movement")
	bool bDecayLaunchSpeed = false;

	UPROPERTY(EditDefaultsOnly, Category="Projectile|Movement", meta=(ClampMin="0.0", UIMin="0.0", EditCondition="bDecayLaunchSpeed", EditConditionHides))
	float LaunchSpeedDecayDuration = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category="Projectile|Movement", meta=(ClampMin="0.0", UIMin="0.0", EditCondition="bDecayLaunchSpeed", EditConditionHides))
	float LaunchSpeedEndScale = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Projectile|Lifetime", meta=(ClampMin="0.0", UIMin="0.0"))
	float PendingDestroyLifeSpan = 0.5f;

	UPROPERTY()
	FOnProjectileHitSignature OnProjectileHit;

	void InitializeProjectile(const FMAProjectileParams& InProjectileParams);

	UPROPERTY(ReplicatedUsing = OnRep_ProjectileVisuals)
	bool bRep_HasElementalVisualData = false;

	UPROPERTY(ReplicatedUsing = OnRep_ProjectileVisuals)
	TObjectPtr<UNiagaraSystem> Rep_MainVFX = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_ProjectileVisuals)
	TObjectPtr<UNiagaraSystem> Rep_TrailVFX = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_ProjectileVisuals)
	FLinearColor Rep_ElementalColor = FLinearColor::White;

	UPROPERTY(ReplicatedUsing = OnRep_ProjectileScale)
	float Rep_SkillAreaScale = 1.f;

	UFUNCTION()
	void OnRep_ProjectileVisuals();

	UFUNCTION()
	void OnRep_ProjectileScale();

private:
	FMAProjectileParams ProjectileParams;

	UPROPERTY(Transient)
	FMASkillScopes EventScopes;

	bool bPendingDestroy = false;
	FVector PreviousHitCheckLocation = FVector::ZeroVector;
	float LaunchSpeed = 0.f;
	float LaunchSpeedDecayElapsed = 0.f;
	bool bLaunchSpeedDecayFinished = false;
	FVector BaseActorScale = FVector::OneVector;
	bool bCapturedBaseActorScale = false;

	FHitResult BuildHitResultFromOverlap(AActor* HitActor, const FHitResult& SweepResult, UPrimitiveComponent* OtherComp) const;
	void ApplyDamageToTarget(UAbilitySystemComponent* TargetASC, const FHitResult& HitResult);
	void ApplySkillAreaScale();
	void ApplyProjectileVisuals();
	void BindHomingTarget();
	void ApplyLaunchSpeedDecay(float DeltaTime);
	void BeginPendingDestroy();
	void ApplyPendingDestroyVisuals();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastBeginPendingDestroy();
	bool CanDamageActor(AActor* OtherActor) const;
	bool TryApplyHitToActor(AActor* OtherActor, const FHitResult& HitResult);
	void CheckContinuousHit();

	TSet<TWeakObjectPtr<AActor>> HitActors;
};
