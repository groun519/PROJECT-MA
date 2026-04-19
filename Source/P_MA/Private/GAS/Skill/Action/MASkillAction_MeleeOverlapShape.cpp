#include "GAS/Skill/Action/MASkillAction_MeleeOverlapShape.h"

#include "DebugShapeHelper.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Skill/Action/MASkillAction_MeleeOverlapHelper.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"
#include "GenericTeamAgentInterface.h"
#include "P_MA/P_MA.h"

namespace
{
	TArray<FHitResult> ResolveShapeHitResults(
		UMASkillAbility& OwnerAbility,
		const FMASkillActionConfig_MeleeOverlapShape& Config,
		const int32 TargetRelationMask,
		FVector& OutCenter)
	{
		UWorld* World = OwnerAbility.GetWorld();
		AActor* AvatarActor = OwnerAbility.GetAvatarActorFromActorInfo();
		if (!World || !AvatarActor)
		{
			OutCenter = FVector::ZeroVector;
			return {};
		}

		const FTransform AvatarTransform = AvatarActor->GetActorTransform();
		OutCenter = AvatarTransform.TransformPosition(Config.LocalOffset);
		const FQuat FinalQuat = AvatarTransform.GetRotation() * Config.LocalRotation.Quaternion();

		FCollisionObjectQueryParams ObjectQueryParams;
		ObjectQueryParams.AddObjectTypesToQuery(ECC_Hitbox);

		FCollisionQueryParams QueryParams;
		if (Config.bIgnoreOwner)
		{
			QueryParams.AddIgnoredActor(AvatarActor);
		}

		const bool bUseSectorFilter = Config.Shape == EVA_Shape::Circle && Config.bUseSector && Config.SectorAngle > 0.f;
		FVector SectorForward = FinalQuat.Vector();
		SectorForward.Z = 0.f;
		if (!SectorForward.Normalize())
		{
			SectorForward = FVector::ForwardVector;
		}

		const float SectorCosThreshold = bUseSectorFilter
			? FMath::Cos(FMath::DegreesToRadians(Config.SectorAngle * 0.5f))
			: -1.f;

		TArray<FOverlapResult> OverlapResults;
		switch (Config.Shape)
		{
		case EVA_Shape::Circle:
			World->OverlapMultiByObjectType(
				OverlapResults,
				OutCenter,
				FQuat::Identity,
				ObjectQueryParams,
				FCollisionShape::MakeSphere(Config.SphereRadius),
				QueryParams);

			if (Config.bDrawDebug)
			{
				FDebugShapeHelper::DrawDebugSectorableCircle(
					World,
					OutCenter,
					Config.SphereRadius,
					32,
					bUseSectorFilter,
					Config.SectorAngle * 0.5f,
					SectorForward,
					FColor::White,
					1.f);
			}
			break;
		case EVA_Shape::Rect:
			World->OverlapMultiByObjectType(
				OverlapResults,
				OutCenter,
				FinalQuat,
				ObjectQueryParams,
				FCollisionShape::MakeBox(Config.BoxHalfSize),
				QueryParams);

			if (Config.bDrawDebug)
			{
				FDebugShapeHelper::DrawDebugRect(
					World,
					OutCenter,
					Config.BoxHalfSize.X,
					Config.BoxHalfSize.Y,
					FinalQuat.Vector(),
					FColor::White,
					1.f);
			}
			break;
		default:
			return {};
		}

		TArray<FOverlapResult> FilteredResults;
		TSet<AActor*> SeenActors;
		IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(AvatarActor);
		for (const FOverlapResult& OverlapResult : OverlapResults)
		{
			AActor* HitActor = OverlapResult.GetActor();
			if (!HitActor || SeenActors.Contains(HitActor))
			{
				continue;
			}

			if (bUseSectorFilter)
			{
				FVector ToTarget = HitActor->GetActorLocation() - OutCenter;
				ToTarget.Z = 0.f;
				if (!ToTarget.Normalize())
				{
					continue;
				}

				if (FVector::DotProduct(SectorForward, ToTarget) < SectorCosThreshold)
				{
					continue;
				}
			}

			if (OwnerTeamInterface)
			{
				const ETeamAttitude::Type TeamAttitude = OwnerTeamInterface->GetTeamAttitudeTowards(*HitActor);
				if (!MATargetRelation::MatchesMask(TargetRelationMask, TeamAttitude))
				{
					continue;
				}
			}

			SeenActors.Add(HitActor);
			FilteredResults.Add(OverlapResult);
		}

		TArray<FHitResult> HitResults;
		FDebugShapeHelper::ConvertOverlapsToHitResults(FilteredResults, HitResults);
		return HitResults;
	}
}

void UMASkillAction_MeleeOverlapShape::Execute(UMASkillAbility& OwnerAbility, FSkillRuntimeContext& RuntimeContext, FMASkillPayloadStore& PayloadStore, const FGameplayEventData& Payload)
{
	(void)Payload;

	if (!OwnerAbility.K2_HasAuthority()) return;

	const FMASkillDamageConfig DamageConfig = MASkillActionMeleeOverlap::ResolveDamageConfig(PayloadStore, DamagePayloadTag);
	const FResolvedSkillHitEffects ResolvedHitEffects = RuntimeContext.BuildResolvedHitEffects(DamageConfig);

	FVector CenterPoint = FVector::ZeroVector;
	const TArray<FHitResult> HitResults = ResolveShapeHitResults(OwnerAbility, Config, ResolvedHitEffects.TargetRelationMask, CenterPoint);
	MASkillActionMeleeOverlap::ApplyHitResults(RuntimeContext, HitResults, ResolvedHitEffects, CenterPoint);
}
