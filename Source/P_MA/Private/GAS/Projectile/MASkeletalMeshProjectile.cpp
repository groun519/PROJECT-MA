#include "GAS/Projectile/MASkeletalMeshProjectile.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

AMASkeletalMeshProjectile::AMASkeletalMeshProjectile()
{
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("SkeletalMesh");
	SkeletalMeshComponent->SetupAttachment(SphereComp);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMeshComponent->SetGenerateOverlapEvents(false);
}

bool AMASkeletalMeshProjectile::SetSkeletalMeshVisual(
	const FMASkeletalMeshProjectileVisualData& VisualData)
{
	if (!HasAuthority() || !VisualData.Mesh) return false;

	Rep_SkeletalMeshVisual = VisualData;
	ApplySkeletalMeshVisual();
	return true;
}

void AMASkeletalMeshProjectile::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AMASkeletalMeshProjectile, Rep_SkeletalMeshVisual, COND_InitialOnly);
}

void AMASkeletalMeshProjectile::OnProjectilePendingDestroy()
{
	if (!SkeletalMeshComponent) return;

	SkeletalMeshComponent->SetVisibility(false, true);
	SkeletalMeshComponent->SetHiddenInGame(true, true);
}

void AMASkeletalMeshProjectile::ApplySkeletalMeshVisual()
{
	if (!SkeletalMeshComponent || !Rep_SkeletalMeshVisual.Mesh) return;

	SkeletalMeshComponent->SetSkeletalMesh(Rep_SkeletalMeshVisual.Mesh);
	SkeletalMeshComponent->SetAbsolute(false, false, true);
	SkeletalMeshComponent->SetWorldScale3D(Rep_SkeletalMeshVisual.WorldScale);

	for (int32 MaterialIndex = 0; MaterialIndex < Rep_SkeletalMeshVisual.Materials.Num(); ++MaterialIndex)
	{
		SkeletalMeshComponent->SetMaterial(MaterialIndex, Rep_SkeletalMeshVisual.Materials[MaterialIndex]);
	}
}
