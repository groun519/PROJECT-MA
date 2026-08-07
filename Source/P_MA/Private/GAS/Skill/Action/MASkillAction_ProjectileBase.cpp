#include "GAS/Skill/Action/MASkillAction_ProjectileBase.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GAS/Projectile/MAProjectileBase.h"
#include "GAS/Skill/Area/MASkillAreaStatics.h"
#include "GAS/Skill/Damage/MASkillDamageResolver.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/MAElementData.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadAccess.h"
#include "GAS/Skill/Runtime/MASkillRuntimeRegistry.h"
#include "GameFramework/Pawn.h"

static bool TryResolveLocationFromObject(UObject* Object, FName SocketName, FVector& OutLocation)
{
	if (USkeletalMeshComponent* MeshComponent = Cast<USkeletalMeshComponent>(Object))
	{
		if (SocketName != NAME_None && MeshComponent->DoesSocketExist(SocketName))
		{
			OutLocation = MeshComponent->GetSocketLocation(SocketName);
			return true;
		}

		OutLocation = MeshComponent->GetComponentLocation();
		return true;
	}

	if (const USceneComponent* SceneComponent = Cast<USceneComponent>(Object))
	{
		OutLocation = SceneComponent->GetComponentLocation();
		return true;
	}

	if (AActor* Actor = Cast<AActor>(Object))
	{
		if (USkeletalMeshComponent* MeshComponent = Actor->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (SocketName != NAME_None && MeshComponent->DoesSocketExist(SocketName))
			{
				OutLocation = MeshComponent->GetSocketLocation(SocketName);
				return true;
			}
		}

		OutLocation = Actor->GetActorLocation();
		return true;
	}

	return false;
}

static bool TryResolveSpawnLocation(AActor& AvatarActor, const FMASkillPayloadAccess& Payloads, const FMASkillActionConfig_SpawnProjectile& Config, FVector& OutLocation)
{
	if (Config.StartObjectSource == EMASkillProjectileStartObjectSource::Self)
	{
		return TryResolveLocationFromObject(&AvatarActor, Config.SpawnSocketName, OutLocation);
	}

	UObject* PayloadObject = nullptr;
	if (!Payloads.Reader.TryGetObject(Config.StartObjectPayloadTag, PayloadObject))
	{
		return false;
	}

	return TryResolveLocationFromObject(PayloadObject, Config.SpawnSocketName, OutLocation);
}

static bool TryResolveProjectileRotation(AActor& AvatarActor, const FMASkillPayloadAccess& Payloads, const FMASkillActionConfig_SpawnProjectile& Config, const FVector& SpawnLocation, FRotator& OutRotation)
{
	if (Config.DirectionSource == EMASkillProjectileDirectionSource::Forward)
	{
		OutRotation = AvatarActor.GetActorRotation();
		return true;
	}

	UObject* DirectionObject = &AvatarActor;
	if (Config.DirectionSource == EMASkillProjectileDirectionSource::ObjectPayload
		&& !Payloads.Reader.TryGetObject(Config.DirectionObjectPayloadTag, DirectionObject))
	{
		return false;
	}

	FVector TargetLocation = FVector::ZeroVector;
	if (!TryResolveLocationFromObject(DirectionObject, NAME_None, TargetLocation))
	{
		return false;
	}

	const FVector Direction = (TargetLocation - SpawnLocation).GetSafeNormal();
	if (Direction.IsNearlyZero()) return false;

	OutRotation = Direction.Rotation();
	return true;
}

static bool TryResolveTargetActor(AActor& AvatarActor, const FMASkillPayloadAccess& Payloads, const FMASkillActionConfig_SpawnProjectile& Config, AActor*& OutTargetActor)
{
	OutTargetActor = nullptr;
	if (!Config.bUseTargetTracking) return true;

	UObject* TargetObject = &AvatarActor;
	if (Config.TargetTracking.TargetSource == EMASkillProjectileTrackingTargetSource::ObjectPayload
		&& !Payloads.Reader.TryGetObject(Config.TargetTracking.TargetObjectPayloadTag, TargetObject))
	{
		return false;
	}

	OutTargetActor = Cast<AActor>(TargetObject);
	return OutTargetActor != nullptr;
}

void UMASkillAction_ProjectileBase::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Ability && Scopes);
	if (!Owner.HasAuthority() || !Config.ProjectileClass) return;

	UWorld* World = Owner.GetWorld();
	if (!World) return;

	const FMASkillPayloadAccess Payloads = Event.GetPayloadAccess(*Scopes);
	if (!Payloads.Reader.IsValid()) return;

	FVector SpawnLocation = FVector::ZeroVector;
	if (!TryResolveSpawnLocation(Owner, Payloads, Config, SpawnLocation)) return;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	if (!TryResolveProjectileRotation(Owner, Payloads, Config, SpawnLocation, SpawnRotation)) return;
	AActor* TargetActor = nullptr;
	if (!TryResolveTargetActor(Owner, Payloads, Config, TargetActor)) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = &Owner;
	SpawnParams.Instigator = Cast<APawn>(&Owner);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMAProjectileBase* Projectile = World->SpawnActor<AMAProjectileBase>(
		Config.ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams);
	if (!Projectile) return;

	FMASkillDamageConfig DamageConfig;
	Payloads.Reader.TryGetStruct(DamagePayloadTag, DamageConfig);

	FMAProjectileParams ProjectileParams;
	ProjectileParams.ResolvedDamage = MASkillDamageResolver::Resolve(*Ability, DamageConfig, Payloads);
	ProjectileParams.ProjectileRadiusMultiplier = MASkillAreaStatics::ResolveAreaScale(
		Payloads,
		Ability->GetAbilitySystemComponentFromActorInfo());
	ProjectileParams.MaxHitCount = Config.MaxHitCount;
	ProjectileParams.ContinuousHitSettings = Config.ContinuousHitSettings;
	ProjectileParams.TargetSettings.TargetActor = TargetActor;
	ProjectileParams.TargetSettings.bHitOnlyTarget = Config.bUseTargetTracking && Config.TargetTracking.bHitOnlyTarget;
	ProjectileParams.EventExecutorAbility = Ability;
	ProjectileParams.EventScopes = *Scopes;

	if (const FMAElementDataRow* ElementRow = FMAElementDataRow::FindByTag(
		Ability->GetVisualElementTag(),
		TEXT("SkillProjectileElementalLookup")))
	{
		ProjectileParams.ElementalSettings.bHasElementalData = true;
		ProjectileParams.ElementalSettings.ElementalColor = ElementRow->ElementColor;
		ProjectileParams.ElementalSettings.MainVFX = ElementRow->MainVFX;
		ProjectileParams.ElementalSettings.TrailVFX = ElementRow->TrailVFX;
	}

	Projectile->InitializeProjectile(ProjectileParams);
	if (!PostSpawnProjectile(*Projectile, Owner, Payloads))
	{
		Projectile->Destroy();
		return;
	}
	Scopes->GetRuntimeRegistry().Register(Projectile);
}

bool UMASkillAction_ProjectileBase::PostSpawnProjectile(AMAProjectileBase& Projectile, AActor& AvatarActor, const FMASkillPayloadAccess& Payloads)
{
	return true;
}
