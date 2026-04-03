#include "GAS/Skill/Action/MASkillAction_SpawnProjectile.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GAS/Projectile/MAProjectile.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"
#include "GameFramework/Pawn.h"

void UMASkillAction_SpawnProjectile::Execute(FSkillRuntimeContext& RuntimeContext, const FGameplayEventData& Payload)
{
	(void)Payload;

	if (!RuntimeContext.HasAuthority() || !Config.ProjectileClass) return;

	UWorld* World = RuntimeContext.GetWorld();
	AActor* AvatarActor = RuntimeContext.GetAvatarActor();
	if (!World || !AvatarActor) return;

	FVector SpawnLocation = AvatarActor->GetActorLocation();
	if (USkeletalMeshComponent* MeshComponent = RuntimeContext.GetOwningMeshComponent())
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

	Projectile->InitializeProjectile(
		RuntimeContext.MakeDamageSpec(&DamageConfig),
		Config.ExplodeRadius,
		Config.bIsPenetrating);
}
