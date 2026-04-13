// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Skill/CrowdControl/MASkillCrowdControlTypes.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "MAProjectile.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UProjectileMovementComponent;
class USphereComponent;

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

	FGameplayTag ElementalTag = FGameplayTag();
	FLinearColor ElementalColor = FLinearColor::White;
};

USTRUCT()
struct P_MA_API FMAProjectileParams
{
	GENERATED_BODY()

	FGameplayEffectSpecHandle DamageSpecHandle;
	TArray<FResolvedCrowdControlEffect> CrowdControlEffects;
	int32 TargetRelationMask = MATargetRelation::GetDefaultMask();
	TObjectPtr<UNiagaraSystem> TrailVFX = nullptr;

	/** Settings **/
	FMAProjectileTargetingSettings TargetingSettings;
	FMAProjectilePenetratingSettings PenetratingSettings;
	FMAProjectileElementalSettings ElementalSettings;
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

	UPROPERTY()
	FOnProjectileHitSignature OnProjectileHit;

	void InitializeProjectile(const FMAProjectileParams& InProjectileParams);
	void SendLocalGameplayCue(const FHitResult& HitResult);

	UPROPERTY(ReplicatedUsing = OnRep_ProjectileVisuals)
	FLinearColor Rep_ElementalColor = FLinearColor::White;

	UFUNCTION()
	void OnRep_ProjectileVisuals();

private:
	FMAProjectileParams ProjectileParams;

	void ApplyEffectSpecsToTarget(UAbilitySystemComponent* TargetASC);
	void ApplyProjectileVisuals();
	FVector ResolveCrowdControlSourcePoint(EMASkillCrowdControlSourceType SourceType) const;
	bool CanDamageActor(AActor* OtherActor) const;
	void CheckAndHandleNearTargetDestroy();

	UPROPERTY(EditDefaultsOnly, Category="Cue Tag", meta=(Categories="GameplayCue"))
	FGameplayTag HitGameplayCueTag;

	UPROPERTY()
	TArray<AActor*> HitActors;
};
