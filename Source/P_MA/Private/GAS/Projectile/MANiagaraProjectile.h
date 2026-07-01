#pragma once

#include "CoreMinimal.h"
#include "GAS/Projectile/MAProjectileBase.h"
#include "MANiagaraProjectile.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class P_MA_API AMANiagaraProjectile : public AMAProjectileBase
{
	GENERATED_BODY()

public:
	AMANiagaraProjectile();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Component")
	TObjectPtr<UNiagaraComponent> Niagara;

	UPROPERTY(EditDefaultsOnly, Category="Projectile|Visual|Main")
	FMAProjectileElementalVisualSettings MainVisualSettings;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void InitializeProjectileVisuals(const FMAProjectileParams& InProjectileParams) override;
	virtual void OnProjectileRadiusChanged() override;
	virtual void OnProjectileElementalColorChanged() override;
	virtual void OnProjectilePendingDestroy() override;

private:
	UPROPERTY(ReplicatedUsing=ApplyProjectileVisuals)
	TObjectPtr<UNiagaraSystem> Rep_MainVFX = nullptr;

	UFUNCTION()
	void ApplyProjectileVisuals();
};
