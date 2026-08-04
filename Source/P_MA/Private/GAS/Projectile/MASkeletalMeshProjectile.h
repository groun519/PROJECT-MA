#pragma once

#include "CoreMinimal.h"
#include "GAS/Projectile/MAProjectileBase.h"
#include "MASkeletalMeshProjectile.generated.h"

class UMaterialInterface;
class USkeletalMesh;
class USkeletalMeshComponent;

USTRUCT()
struct P_MA_API FMASkeletalMeshProjectileVisualData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<USkeletalMesh> Mesh = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInterface>> Materials;

	UPROPERTY()
	FVector WorldScale = FVector::OneVector;
};

UCLASS()
class P_MA_API AMASkeletalMeshProjectile : public AMAProjectileBase
{
	GENERATED_BODY()

public:
	AMASkeletalMeshProjectile();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Component")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	bool SetSkeletalMeshVisual(const FMASkeletalMeshProjectileVisualData& VisualData);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnProjectilePendingDestroy() override;

private:
	UPROPERTY(ReplicatedUsing=ApplySkeletalMeshVisual)
	FMASkeletalMeshProjectileVisualData Rep_SkeletalMeshVisual;

	UFUNCTION()
	void ApplySkeletalMeshVisual();
};
