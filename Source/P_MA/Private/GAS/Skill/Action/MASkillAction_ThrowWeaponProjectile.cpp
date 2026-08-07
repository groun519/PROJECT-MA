#include "GAS/Skill/Action/MASkillAction_ThrowWeaponProjectile.h"

#include "GAS/Projectile/MASkeletalMeshProjectile.h"
#include "Weapon/WeaponComponent.h"

bool UMASkillAction_ThrowWeaponProjectile::PostSpawnProjectile(
	AMAProjectileBase& Projectile,
	AActor& AvatarActor,
	const FMASkillPayloadAccess& Payloads)
{
	const UWeaponComponent* SourceWeapon = AvatarActor.FindComponentByClass<UWeaponComponent>();
	AMASkeletalMeshProjectile* SkeletalMeshProjectile = Cast<AMASkeletalMeshProjectile>(&Projectile);
	if (!SourceWeapon || !SkeletalMeshProjectile) return false;

	FMASkeletalMeshProjectileVisualData VisualData;
	VisualData.Mesh = SourceWeapon->GetSkeletalMeshAsset();
	VisualData.WorldScale = SourceWeapon->GetComponentScale();
	VisualData.Materials.Reserve(SourceWeapon->GetNumMaterials());
	for (int32 MaterialIndex = 0; MaterialIndex < SourceWeapon->GetNumMaterials(); ++MaterialIndex)
	{
		VisualData.Materials.Add(SourceWeapon->GetMaterial(MaterialIndex));
	}

	return SkeletalMeshProjectile->SetSkeletalMeshVisual(VisualData);
}
