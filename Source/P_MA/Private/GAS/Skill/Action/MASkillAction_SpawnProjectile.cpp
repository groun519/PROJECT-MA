#include "GAS/Skill/Action/MASkillAction_SpawnProjectile.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GAS/Projectile/MAProjectile.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"
#include "GameFramework/Pawn.h"

void UMASkillAction_SpawnProjectile::Execute(UMASkillAbility& OwnerAbility, FSkillRuntimeContext& RuntimeContext, const FGameplayEventData& Payload)
{
	(void)OwnerAbility;
	(void)Payload;

	if (!OwnerAbility.K2_HasAuthority() || !Config.ProjectileClass) return;

	UWorld* World = OwnerAbility.GetWorld();
	AActor* AvatarActor = OwnerAbility.GetAvatarActorFromActorInfo();
	if (!World || !AvatarActor) return;

	FVector SpawnLocation = AvatarActor->GetActorLocation();
	if (USkeletalMeshComponent* MeshComponent = OwnerAbility.GetOwningComponentFromActorInfo())
	{
		if (Config.SpawnSocketName != NAME_None && MeshComponent->DoesSocketExist(Config.SpawnSocketName))
		{
			SpawnLocation = MeshComponent->GetSocketLocation(Config.SpawnSocketName);
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = AvatarActor;
	SpawnParams.Instigator = Cast<APawn>(AvatarActor);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMAProjectile* Projectile = World->SpawnActor<AMAProjectile>(
		Config.ProjectileClass,
		SpawnLocation,
		AvatarActor->GetActorRotation(),
		SpawnParams);
	if (!Projectile) return;

	const FResolvedSkillHitEffects ResolvedHitEffects = RuntimeContext.BuildResolvedHitEffects(&DamageConfig);
	Projectile->InitializeProjectile(
		ResolvedHitEffects.DamageSpec,
		Config.ExplodeRadius,
		Config.bIsPenetrating,
		ResolvedHitEffects.CrowdControlEffects,
		ResolvedHitEffects.TargetRelationMask);
}
