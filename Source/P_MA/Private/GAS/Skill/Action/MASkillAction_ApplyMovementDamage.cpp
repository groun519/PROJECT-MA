#include "GAS/Skill/Action/MASkillAction_ApplyMovementDamage.h"

#include "Character/MACharacter.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Skill/Damage/MASkillDamageApplicator.h"
#include "GAS/Skill/Damage/MASkillDamageResolver.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadAccessor.h"
#include "GAS/Skill/Runtime/MASkillRuntimeRegistry.h"
#include "GameFramework/Pawn.h"
#include "GenericTeamAgentInterface.h"
#include "P_MA/P_MA.h"

AMASkillMovementDamageRuntime::AMASkillMovementDamageRuntime()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;
	SetReplicates(false);
	SetActorEnableCollision(false);
}

bool AMASkillMovementDamageRuntime::Initialize(
	UMASkillAbility& InOwnerAbility,
	const FMASkillScopes& InScopes,
	const FMAActionImpulseHandle& InMovementHandle,
	const FResolvedSkillDamage& InResolvedDamage,
	float InCapsuleRadiusScale,
	float InCapsuleHalfHeightScale)
{
	AMACharacter* Character = Cast<AMACharacter>(InOwnerAbility.GetAvatarActorFromActorInfo());
	UMAImpulseComponent* CharacterImpulseComponent = Character ? Character->GetImpulseComponent() : nullptr;
	if (!Character || !CharacterImpulseComponent || !CharacterImpulseComponent->IsActionImpulseActive(InMovementHandle))
	{
		return false;
	}

	OwnerAbility = &InOwnerAbility;
	OwnerCharacter = Character;
	ImpulseComponent = CharacterImpulseComponent;
	Scopes = InScopes;
	MovementHandle = InMovementHandle;
	ResolvedDamage = InResolvedDamage;
	CapsuleRadiusScale = FMath::Max(InCapsuleRadiusScale, 0.01f);
	CapsuleHalfHeightScale = FMath::Max(InCapsuleHalfHeightScale, 0.01f);
	PreviousLocation = Character->GetActorLocation();
	SweepMovement(PreviousLocation, PreviousLocation);
	return true;
}

void AMASkillMovementDamageRuntime::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AMACharacter* Character = OwnerCharacter.Get();
	UMAImpulseComponent* CharacterImpulseComponent = ImpulseComponent.Get();
	if (!Character || !OwnerAbility.IsValid() || !CharacterImpulseComponent)
	{
		Destroy();
		return;
	}

	const FVector CurrentLocation = Character->GetActorLocation();
	SweepMovement(PreviousLocation, CurrentLocation);
	PreviousLocation = CurrentLocation;

	if (!CharacterImpulseComponent->IsActionImpulseActive(MovementHandle))
	{
		Destroy();
	}
}

void AMASkillMovementDamageRuntime::SweepMovement(const FVector& Start, const FVector& End)
{
	UMASkillAbility* SkillAbility = OwnerAbility.Get();
	AMACharacter* Character = OwnerCharacter.Get();
	UCapsuleComponent* Capsule = Character ? Character->GetCapsuleComponent() : nullptr;
	UWorld* World = GetWorld();
	if (!SkillAbility || !Character || !Capsule || !World) return;

	const float Radius = Capsule->GetScaledCapsuleRadius() * CapsuleRadiusScale;
	const float HalfHeight = FMath::Max(
		Radius,
		Capsule->GetScaledCapsuleHalfHeight() * CapsuleHalfHeightScale);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Hitbox);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MASkillMovementDamage), false, this);
	QueryParams.AddIgnoredActor(this);
	if (!MATargetRelation::IncludesSelf(ResolvedDamage.TargetRelationMask))
	{
		QueryParams.AddIgnoredActor(Character);
	}

	TArray<FHitResult> SweepHits;
	if (!World->SweepMultiByObjectType(
		SweepHits,
		Start,
		End,
		Character->GetActorQuat(),
		ObjectQueryParams,
		FCollisionShape::MakeCapsule(Radius, HalfHeight),
		QueryParams))
	{
		return;
	}

	IGenericTeamAgentInterface* SourceTeam = Cast<IGenericTeamAgentInterface>(Character);
	for (const FHitResult& SweepHit : SweepHits)
	{
		AActor* HitActor = SweepHit.GetActor();
		const TWeakObjectPtr<AActor> HitActorKey = HitActor;
		if (!HitActor || HitActors.Contains(HitActorKey)) continue;

		if (SourceTeam)
		{
			const ETeamAttitude::Type TeamAttitude = SourceTeam->GetTeamAttitudeTowards(*HitActor);
			if (!MATargetRelation::MatchesTarget(
				ResolvedDamage.TargetRelationMask,
				Character,
				HitActor,
				TeamAttitude))
			{
				continue;
			}
		}
		else if (MATargetRelation::IsSelfTarget(Character, HitActor)
			&& !MATargetRelation::IncludesSelf(ResolvedDamage.TargetRelationMask))
		{
			continue;
		}

		MASkillDamageApplicator::ApplyHitResult(
			*SkillAbility,
			Scopes,
			SweepHit,
			ResolvedDamage,
			Character->GetActorLocation());
		HitActors.Add(HitActorKey);
	}
}

void UMASkillAction_ApplyMovementDamage::Execute(
	UMASkillAbility& OwnerAbility,
	const FMASkillEvent& Event,
	const FMASkillScopes& Scopes)
{
	if (!OwnerAbility.K2_HasAuthority()) return;

	FMAActionImpulseHandle MovementHandle;
	if (!Event.Payloads.TryGetStruct(
		UMAAbilitySystemStatics::GetMovementHandleTag(),
		MovementHandle))
	{
		return;
	}

	const FMASkillPayloadAccessor Payloads = Event.GetPayloadAccess(Scopes);
	FMASkillDamageConfig DamageConfig;
	if (!Payloads.TryGetStruct(DamagePayloadTag, DamageConfig)) return;

	UWorld* World = OwnerAbility.GetWorld();
	AActor* AvatarActor = OwnerAbility.GetAvatarActorFromActorInfo();
	if (!World || !AvatarActor) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = AvatarActor;
	SpawnParams.Instigator = Cast<APawn>(AvatarActor);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMASkillMovementDamageRuntime* Runtime = World->SpawnActor<AMASkillMovementDamageRuntime>(
		AvatarActor->GetActorLocation(),
		FRotator::ZeroRotator,
		SpawnParams);
	if (!Runtime) return;

	if (!Runtime->Initialize(
		OwnerAbility,
		Scopes,
		MovementHandle,
		MASkillDamageResolver::Resolve(OwnerAbility, DamageConfig, Payloads),
		CapsuleRadiusScale,
		CapsuleHalfHeightScale))
	{
		Runtime->Destroy();
		return;
	}

	Scopes.GetRuntimeRegistry().Register(Runtime);
}
