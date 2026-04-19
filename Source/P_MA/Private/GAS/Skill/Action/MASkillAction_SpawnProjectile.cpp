#include "GAS/Skill/Action/MASkillAction_SpawnProjectile.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GAS/Projectile/MAProjectile.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/MAElementData.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"
#include "GameFramework/Pawn.h"

void UMASkillAction_SpawnProjectile::Execute(UMASkillAbility& OwnerAbility, FSkillRuntimeContext& RuntimeContext, FMASkillPayloadStore& PayloadStore, const FGameplayEventData& Payload)
{
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

	FMASkillDamageConfig DamageConfig;
	PayloadStore.TryGetStruct(DamagePayloadTag, DamageConfig);

	const FResolvedSkillHitEffects ResolvedHitEffects = RuntimeContext.BuildResolvedHitEffects(DamageConfig);
	FMAProjectileParams ProjectileParams;
	ProjectileParams.DamageSpecHandle = ResolvedHitEffects.DamageSpec;
	ProjectileParams.CrowdControlEffects = ResolvedHitEffects.CrowdControlEffects;
	ProjectileParams.TargetRelationMask = ResolvedHitEffects.TargetRelationMask;
	ProjectileParams.PenetratingSettings.bIsPenetrating = Config.bIsPenetrating;
	ProjectileParams.ElementalSettings.ElementalTag = OwnerAbility.GetElementalTag();

	if (const UDataTable* ElementalDataTable = OwnerAbility.GetElementalDataTable();
		ElementalDataTable && ProjectileParams.ElementalSettings.ElementalTag.IsValid())
	{
		FString ElementalRowNameString = ProjectileParams.ElementalSettings.ElementalTag.GetTagName().ToString();
		ElementalRowNameString.Split(TEXT("."), nullptr, &ElementalRowNameString, ESearchCase::CaseSensitive, ESearchDir::FromEnd);

		if (const FMAElementDataRow* ElementRow = ElementalDataTable->FindRow<FMAElementDataRow>(
			FName(*ElementalRowNameString),
			TEXT("SkillProjectileElementalLookup")))
		{
			ProjectileParams.ElementalSettings.ElementalColor = ElementRow->ElementColor;
			ProjectileParams.ElementalSettings.HitGameplayCueTag = ElementRow->HitGameplayCueTag;
			ProjectileParams.TrailVFX = ElementRow->TrailVFX;
		}
	}

	Projectile->InitializeProjectile(ProjectileParams);
}
