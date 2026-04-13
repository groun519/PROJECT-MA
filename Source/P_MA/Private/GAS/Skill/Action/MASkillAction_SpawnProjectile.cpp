#include "GAS/Skill/Action/MASkillAction_SpawnProjectile.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GAS/Projectile/MAProjectile.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/MAElementData.h"
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
	FMAProjectileParams ProjectileParams;
	ProjectileParams.DamageSpecHandle = ResolvedHitEffects.DamageSpec;
	ProjectileParams.CrowdControlEffects = ResolvedHitEffects.CrowdControlEffects;
	ProjectileParams.TargetRelationMask = ResolvedHitEffects.TargetRelationMask;
	ProjectileParams.PenetratingSettings.bIsPenetrating = Config.bIsPenetrating;
	ProjectileParams.ElementalSettings.ElementalTag = OwnerAbility.GetElementalTag();

	if (const UDataTable* ElementalDataTable = OwnerAbility.GetElementalDataTable())
	{
		TArray<FMAElementDataRow*> ElementRows;
		ElementalDataTable->GetAllRows(TEXT("SkillProjectileElementalLookup"), ElementRows);
		for (const FMAElementDataRow* ElementRow : ElementRows)
		{
			if (!ElementRow || ElementRow->ElementTag != ProjectileParams.ElementalSettings.ElementalTag) continue;
			ProjectileParams.ElementalSettings.ElementalColor = ElementRow->ElementColor;
			break;
		}
	}

	Projectile->InitializeProjectile(ProjectileParams);
}
