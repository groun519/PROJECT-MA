#include "GAS/Skill/Action/MASkillAction_ProjectileBase.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GAS/Projectile/MAProjectile.h"
#include "GAS/Skill/Area/MASkillAreaStatics.h"
#include "GAS/Skill/Damage/MASkillDamageResolver.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/MAElementData.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadAccessor.h"
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

static bool TryResolveSpawnLocation(AActor& AvatarActor, const FMASkillPayloadAccessor& Payloads, const FMASkillActionConfig_SpawnProjectile& Config, FVector& OutLocation)
{
	if (Config.StartObjectSource == EMASkillProjectileStartObjectSource::Self)
	{
		return TryResolveLocationFromObject(&AvatarActor, Config.SpawnSocketName, OutLocation);
	}

	UObject* PayloadObject = nullptr;
	if (!Payloads.TryGetObject(Config.StartObjectPayloadTag, PayloadObject))
	{
		return false;
	}

	return TryResolveLocationFromObject(PayloadObject, Config.SpawnSocketName, OutLocation);
}

static bool TryResolveProjectileRotation(AActor& AvatarActor, const FMASkillPayloadAccessor& Payloads, const FMASkillActionConfig_SpawnProjectile& Config, const FVector& SpawnLocation, FRotator& OutRotation)
{
	if (Config.DirectionSource == EMASkillProjectileDirectionSource::Forward)
	{
		OutRotation = AvatarActor.GetActorRotation();
		return true;
	}

	UObject* DirectionObject = &AvatarActor;
	if (Config.DirectionSource == EMASkillProjectileDirectionSource::ObjectPayload
		&& !Payloads.TryGetObject(Config.DirectionObjectPayloadTag, DirectionObject))
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

static bool TryResolveTargetActor(AActor& AvatarActor, const FMASkillPayloadAccessor& Payloads, const FMASkillActionConfig_SpawnProjectile& Config, AActor*& OutTargetActor)
{
	OutTargetActor = nullptr;
	if (!Config.bUseTargetTracking) return true;

	UObject* TargetObject = &AvatarActor;
	if (Config.TargetTracking.TargetSource == EMASkillProjectileTrackingTargetSource::ObjectPayload
		&& !Payloads.TryGetObject(Config.TargetTracking.TargetObjectPayloadTag, TargetObject))
	{
		return false;
	}

	OutTargetActor = Cast<AActor>(TargetObject);
	return OutTargetActor != nullptr;
}

void UMASkillAction_ProjectileBase::Execute(
	UMASkillAbility& OwnerAbility,
	const FMASkillEvent& Event,
	const FMASkillScopes& Scopes)
{
	if (!OwnerAbility.K2_HasAuthority() || !Config.ProjectileClass) return;

	UWorld* World = OwnerAbility.GetWorld();
	AActor* AvatarActor = OwnerAbility.GetAvatarActorFromActorInfo();
	if (!World || !AvatarActor) return;

	const FMASkillPayloadAccessor Payloads = Event.GetPayloadAccess(Scopes);
	if (!Payloads.IsValid()) return;

	FVector SpawnLocation = FVector::ZeroVector;
	if (!TryResolveSpawnLocation(*AvatarActor, Payloads, Config, SpawnLocation)) return;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	if (!TryResolveProjectileRotation(*AvatarActor, Payloads, Config, SpawnLocation, SpawnRotation)) return;
	AActor* TargetActor = nullptr;
	if (!TryResolveTargetActor(*AvatarActor, Payloads, Config, TargetActor)) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = AvatarActor;
	SpawnParams.Instigator = Cast<APawn>(AvatarActor);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMAProjectile* Projectile = World->SpawnActor<AMAProjectile>(
		Config.ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams);
	if (!Projectile) return;

	if (!PostSpawnProjectile(*Projectile, *AvatarActor, Payloads))
	{
		Projectile->Destroy();
		return;
	}

	FMASkillDamageConfig DamageConfig;
	Payloads.TryGetStruct(DamagePayloadTag, DamageConfig);

	FMAProjectileParams ProjectileParams;
	ProjectileParams.ResolvedDamage = MASkillDamageResolver::Resolve(OwnerAbility, DamageConfig, Payloads);
	ProjectileParams.SkillAreaScale = MASkillAreaStatics::ResolveAreaScale(Payloads);
	ProjectileParams.PenetratingSettings.bIsPenetrating = Config.bIsPenetrating;
	ProjectileParams.ContinuousHitSettings = Config.ContinuousHitSettings;
	ProjectileParams.TargetSettings.TargetActor = TargetActor;
	ProjectileParams.TargetSettings.bHitOnlyTarget = Config.bUseTargetTracking && Config.TargetTracking.bHitOnlyTarget;
	ProjectileParams.EventExecutorAbility = &OwnerAbility;
	ProjectileParams.EventScopes = Scopes;

	const FGameplayTag ElementalTag = OwnerAbility.GetElementalTag();
	if (const UDataTable* ElementalDataTable = OwnerAbility.GetElementalDataTable();
		ElementalDataTable && ElementalTag.IsValid())
	{
		FString ElementalRowNameString = ElementalTag.GetTagName().ToString();
		ElementalRowNameString.Split(TEXT("."), nullptr, &ElementalRowNameString, ESearchCase::CaseSensitive, ESearchDir::FromEnd);

		if (const FMAElementDataRow* ElementRow = ElementalDataTable->FindRow<FMAElementDataRow>(
			FName(*ElementalRowNameString),
			TEXT("SkillProjectileElementalLookup")))
		{
			ProjectileParams.ElementalSettings.bHasElementalData = true;
			ProjectileParams.ElementalSettings.ElementalColor = ElementRow->ElementColor;
			ProjectileParams.ElementalSettings.MainVFX = ElementRow->MainVFX;
			ProjectileParams.ElementalSettings.TrailVFX = ElementRow->TrailVFX;
		}
	}

	Projectile->InitializeProjectile(ProjectileParams);
	Scopes.GetRuntimeRegistry().Register(Projectile);
}

bool UMASkillAction_ProjectileBase::PostSpawnProjectile(AMAProjectile& Projectile, AActor& AvatarActor, const FMASkillPayloadAccessor& Payloads)
{
	return true;
}
