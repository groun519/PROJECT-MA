#include "GAS/Skill/Action/MASkillAction_SpawnProjectile.h"

#include "GAS/Projectile/MAProjectile.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"

void UMASkillAction_SpawnProjectile::Execute(UMASkillAbility* SkillAbility, FSkillRuntimeContext& RuntimeContext, const FGameplayEventData& Payload)
{
	if (!SkillAbility || !SkillAbility->K2_HasAuthority() || !Config.ProjectileClass)
	{
		return;
	}

	AActor* AvatarActor = SkillAbility->GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return;
	}

	FVector SpawnLocation = AvatarActor->GetActorLocation();
	if (USkeletalMeshComponent* MeshComponent = SkillAbility->GetOwningComponentFromActorInfo())
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

	AMAProjectile* Projectile = SkillAbility->GetWorld()->SpawnActor<AMAProjectile>(Config.ProjectileClass, SpawnLocation, AvatarActor->GetActorRotation(), SpawnParams);
	if (!Projectile)
	{
		return;
	}

	const UMASkillDefinition* SkillDefinition = SkillAbility->GetSkillDefinition();
	const TSubclassOf<UGameplayEffect> ResolvedDamageEffect = SkillDefinition ? SkillDefinition->GetDefaultDamageEffect() : nullptr;
	const int32 AbilityLevel = SkillAbility->GetAbilityLevel(SkillAbility->GetCurrentAbilitySpecHandle(), SkillAbility->GetCurrentActorInfo());
	FGameplayEffectSpecHandle DamageSpecHandle;

	if (ResolvedDamageEffect)
	{
		DamageSpecHandle = SkillAbility->MakeOutgoingGameplayEffectSpec(ResolvedDamageEffect, AbilityLevel);
	}

	Projectile->InitializeProjectile(DamageSpecHandle, Config.ExplodeRadius, Config.bIsPenetrating);
}
