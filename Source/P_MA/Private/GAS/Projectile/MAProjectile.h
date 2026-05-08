#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GameFramework/Actor.h"
#include "MAProjectile.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UProjectileMovementComponent;
class USphereComponent;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProjectileHitSignature, AActor*, HitActor);

USTRUCT()
struct P_MA_API FMAProjectileTargetingSettings
{
	GENERATED_BODY()

	bool bHitOnlyDamageTarget = false;
	TWeakObjectPtr<AActor> DamageTarget;
};

USTRUCT()
struct P_MA_API FMAProjectilePenetratingSettings
{
	GENERATED_BODY()

	bool bIsPenetrating = false;
	int32 PenetratingCount = 0;
};

USTRUCT()
struct P_MA_API FMAProjectileElementalSettings
{
	GENERATED_BODY()

	FGameplayTag ElementalTag;
	FLinearColor ElementalColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct P_MA_API FMAProjectileContinuousHitSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Continuous Hit")
	bool bEnabled = true;

	UPROPERTY(EditDefaultsOnly, Category="Continuous Hit", meta=(ClampMin="0.01", UIMin="0.01"))
	float TickInterval = 0.08f;

	UPROPERTY(EditDefaultsOnly, Category="Continuous Hit", meta=(ClampMin="0.0", UIMin="0.0"))
	float MinSweepDistance = 10.f;

	UPROPERTY(EditDefaultsOnly, Category="Continuous Hit", meta=(ClampMin="1.0", UIMin="1.0"))
	float MaxSweepSegmentLength = 150.f;

	UPROPERTY(EditDefaultsOnly, Category="Continuous Hit", meta=(ClampMin="1", UIMin="1"))
	int32 MaxSweepSubsteps = 8;
};

USTRUCT()
struct P_MA_API FMAProjectileParams
{
	GENERATED_BODY()

	FResolvedSkillHitEffects ResolvedHitEffects;
	TObjectPtr<UNiagaraSystem> TrailVFX = nullptr;

	/** Settings **/
	FMAProjectileTargetingSettings TargetingSettings;
	FMAProjectilePenetratingSettings PenetratingSettings;
	FMAProjectileElementalSettings ElementalSettings;
	FMAProjectileContinuousHitSettings ContinuousHitSettings;
};

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

	UPROPERTY()
	FOnProjectileHitSignature OnProjectileHit;

	void InitializeProjectile(const FMAProjectileParams& InProjectileParams);

	UPROPERTY(ReplicatedUsing = OnRep_ProjectileVisuals)
	TObjectPtr<UNiagaraSystem> Rep_TrailVFX = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_ProjectileVisuals)
	FLinearColor Rep_ElementalColor = FLinearColor::White;

	UFUNCTION()
	void OnRep_ProjectileVisuals();

private:
	FMAProjectileParams ProjectileParams;
	bool bPendingDestroy = false;
	FVector PreviousHitCheckLocation = FVector::ZeroVector;

	FHitResult BuildHitResultFromOverlap(AActor* HitActor, const FHitResult& SweepResult, UPrimitiveComponent* OtherComp) const;
	FHitResult BuildHitResultFromActor(AActor* HitActor) const;
	void ApplyResolvedHitEffectsToTarget(UAbilitySystemComponent* TargetASC, const FHitResult& HitResult);
	void ApplyProjectileVisuals();
	void BeginPendingDestroy();
	void ApplyPendingDestroyVisuals();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastBeginPendingDestroy();
	bool CanDamageActor(AActor* OtherActor) const;
	bool TryApplyHitToActor(AActor* OtherActor, const FHitResult& HitResult);
	void CheckContinuousHit();
	void CheckAndHandleNearTargetHit();

	TSet<TWeakObjectPtr<AActor>> HitActors;
};
