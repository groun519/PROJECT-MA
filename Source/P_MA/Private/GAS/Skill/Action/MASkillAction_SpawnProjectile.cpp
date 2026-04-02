#include "GAS/Skill/Action/MASkillAction_SpawnProjectile.h"

#include "Components/SkeletalMeshComponent.h"
#include "GAS/Projectile/MAProjectile.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"

void UMASkillAction_SpawnProjectile::Execute(FSkillRuntimeContext& RuntimeContext, const FGameplayEventData& Payload)
{
	(void)Payload;

	if (!RuntimeContext.HasAuthority() || !Config.ProjectileClass) return;

	AActor* AvatarActor = RuntimeContext.GetAvatarActor();
	if (!AvatarActor) return;

	FVector SpawnLocation = AvatarActor->GetActorLocation();
	if (USkeletalMeshComponent* MeshComponent = RuntimeContext.GetOwningMeshComponent())
	{
		if (Config.SpawnSocketName != NAME_None && MeshComponent->DoesSocketExist(Config.SpawnSocketName))
		{
			SpawnLocation = MeshComponent->GetSocketLocation(Config.SpawnSocketName);
		}
	}

	AMAProjectile* Projectile = RuntimeContext.SpawnDamageProjectile(
		Config.ProjectileClass,
		SpawnLocation,
		AvatarActor->GetActorRotation(),
		&DamageConfig,
		Config.ExplodeRadius,
		Config.bIsPenetrating);
	if (!Projectile) return;
}
