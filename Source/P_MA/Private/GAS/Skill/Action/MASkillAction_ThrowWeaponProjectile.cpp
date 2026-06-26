#include "GAS/Skill/Action/MASkillAction_ThrowWeaponProjectile.h"

#include "Components/SkeletalMeshComponent.h"
#include "GAS/Projectile/MAProjectile.h"
#include "Weapon/WeaponComponent.h"

static USkeletalMeshComponent* FindProjectileWeaponMesh(AMAProjectile& Projectile, FName ComponentName)
{
	if (ComponentName == NAME_None) return Projectile.FindComponentByClass<USkeletalMeshComponent>();

	TInlineComponentArray<USkeletalMeshComponent*> MeshComponents(&Projectile);
	for (USkeletalMeshComponent* MeshComponent : MeshComponents)
	{
		if (MeshComponent && MeshComponent->GetFName() == ComponentName)
		{
			return MeshComponent;
		}
	}

	return nullptr;
}

bool UMASkillAction_ThrowWeaponProjectile::PostSpawnProjectile(
	AMAProjectile& Projectile,
	AActor& AvatarActor,
	const FMASkillPayloadAccessor& Payloads)
{
	const UWeaponComponent* SourceWeapon = AvatarActor.FindComponentByClass<UWeaponComponent>();
	USkeletalMeshComponent* ProjectileWeaponMesh = FindProjectileWeaponMesh(Projectile, ProjectileWeaponMeshComponentName);
	if (!SourceWeapon || !ProjectileWeaponMesh) return false;

	ProjectileWeaponMesh->SetSkeletalMesh(SourceWeapon->GetSkeletalMeshAsset());
	for (int32 MaterialIndex = 0; MaterialIndex < SourceWeapon->GetNumMaterials(); ++MaterialIndex)
	{
		ProjectileWeaponMesh->SetMaterial(MaterialIndex, SourceWeapon->GetMaterial(MaterialIndex));
	}
	return true;
}
