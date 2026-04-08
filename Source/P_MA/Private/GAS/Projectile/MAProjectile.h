// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Skill/CrowdControl/MASkillCrowdControlResolvedTypes.h"
#include "GameplayEffectTypes.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Actor.h"
#include "MAProjectile.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UProjectileMovementComponent;
class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProjectileHitSignature, AActor*, HitActor);

UCLASS()
class AMAProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AMAProjectile();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Component")
	TObjectPtr<USphereComponent> SphereComp;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Component")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Component")
	TObjectPtr<UNiagaraComponent> Niagara;

	void SetGameplayCueTag(FGameplayTag Tag);
	void SetProjectileVFX(UNiagaraSystem* NewVFX);

	UPROPERTY()
	FOnProjectileHitSignature OnProjectileHit;
	

	/** Targeting Logics **/
	void SetDamageTarget(AActor* InTarget);
	void SetHitOnlyDamageTargetEnabled(bool bInEnabled);
	/**/

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	FGameplayEffectSpecHandle DamageEffectSpecHandle;
	TArray<FResolvedCrowdControlEffect> AdditionalCrowdControlEffects;

	void SendLocalGameplayCue(const FHitResult& HitResult);

	UPROPERTY(ReplicatedUsing = OnRep_ProjectileVFX)
	TObjectPtr<UNiagaraSystem> Rep_ProjectileVFX;

	UFUNCTION()
	void OnRep_ProjectileVFX();
public:	
	virtual void InitializeProjectile(const FGameplayEffectSpecHandle& InSpecHandle, float InExplodeRadius, bool bInPenetrate = false, const TArray<FResolvedCrowdControlEffect>& InAdditionalCrowdControlEffects = {}, int32 InTargetRelationMask = MATargetRelation::GetDefaultMask());


private:
	void ApplyEffectSpecsToTarget(UAbilitySystemComponent* TargetASC);
	FVector ResolveCrowdControlSourcePoint(EMASkillCrowdControlSourceType SourceType) const;
	bool CanDamageActor(AActor* OtherActor) const;
	void CheckAndHandleNearTargetDestroy();

	UPROPERTY()
	float ExplodeRadius;

	bool bIsPenetrating = false;

	UPROPERTY(EditDefaultsOnly, Category="Cue Tag", meta=(Categories="GameplayCue"))
	FGameplayTag HitGameplayCueTag;

	UPROPERTY()
	TArray<AActor*> HitActors;

	/** Targeting **/
	UPROPERTY()
	TWeakObjectPtr<AActor> DamageTarget;
	UPROPERTY()
	bool bHitOnlyDamageTarget = false;

	UPROPERTY()
	int32 DamageTargetRelationMask = MATargetRelation::GetDefaultMask();
};
