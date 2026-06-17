#include "GAS/Skill/Area/MASkillAreaStatics.h"

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Skill/Area/Debug/DebugShapeHelper.h"
#include "GAS/Skill/Area/MASkillAreaTargetData.h"
#include "GAS/Skill/Area/MASkillAreaTypes.h"
#include "GenericTeamAgentInterface.h"
#include "P_MA/P_MA.h"

static void ResolveOverlapResults(
	UWorld& World,
	const FMASkillWorldAreaShape& Area,
	const FMASkillWorldCircleArea& Circle,
	const FCollisionObjectQueryParams& ObjectQueryParams,
	const FCollisionQueryParams& QueryParams,
	TArray<FOverlapResult>& OutResults)
{
	World.OverlapMultiByObjectType(
		OutResults,
		Area.Center,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(Circle.Radius),
		QueryParams);
}

static void ResolveOverlapResults(
	UWorld& World,
	const FMASkillWorldAreaShape& Area,
	const FMASkillWorldRectArea& Rect,
	const FCollisionObjectQueryParams& ObjectQueryParams,
	const FCollisionQueryParams& QueryParams,
	TArray<FOverlapResult>& OutResults)
{
	World.OverlapMultiByObjectType(
		OutResults,
		Area.Center,
		Area.Rotation.Quaternion(),
		ObjectQueryParams,
		FCollisionShape::MakeBox(FVector(Rect.Height * 0.5f, Rect.Width * 0.5f, Rect.Depth * 0.5f)),
		QueryParams);
}

static void DrawWorldPreview(
	UWorld& World,
	const FMASkillWorldAreaShape& Area,
	const FMASkillWorldCircleArea& Circle)
{
	const bool bUseSector = Circle.bUseSector && Circle.SectorAngle > 0.f && Circle.SectorAngle < 360.f;
	FDebugShapeHelper::DrawDebugSectorableCircle(
		&World,
		Area.Center,
		Circle.Radius,
		bUseSector ? 360 : 32,
		bUseSector,
		bUseSector ? Circle.SectorAngle * 0.5f : 0.f,
		Area.GetForward(),
		Area.DebugColor,
		Area.DebugThickness);
}

static void DrawWorldPreview(
	UWorld& World,
	const FMASkillWorldAreaShape& Area,
	const FMASkillWorldRectArea& Rect)
{
	FDebugShapeHelper::DrawDebugRect(
		&World,
		Area.Center,
		Rect.Height * 0.5f,
		Rect.Width * 0.5f,
		Area.GetForward(),
		Area.DebugColor,
		Area.DebugThickness);
}

static void DrawWorldPreview(
	UWorld& World,
	const FMASkillWorldAreaShape& Area,
	const FMASkillWorldLineArea& Line)
{
	FDebugShapeHelper::DrawDebugRect(
		&World,
		Area.Center,
		Line.Length * 0.5f,
		Line.Width * 0.5f,
		Area.GetForward(),
		Area.DebugColor,
		Area.DebugThickness);
}

const FMASkillWorldAreaShape* MASkillAreaStatics::FindWorldShape(const FGameplayAbilityTargetDataHandle& TargetData)
{
	for (const TSharedPtr<FGameplayAbilityTargetData>& Data : TargetData.Data)
	{
		if (Data.IsValid() && Data->GetScriptStruct() == FGameplayAbilityTargetData_SkillArea::StaticStruct())
		{
			return &static_cast<const FGameplayAbilityTargetData_SkillArea*>(Data.Get())->Area;
		}
	}
	return nullptr;
}

TArray<FHitResult> MASkillAreaStatics::ResolveHitResults(
	UWorld& World,
	AActor* SourceActor,
	const FMASkillWorldAreaShape& Area,
	int32 TargetRelationMask)
{
	if (!SourceActor || !Area.IsValid() || !Area.CanResolveHit()) return {};

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Hitbox);

	FCollisionQueryParams QueryParams;
	if (Area.bIgnoreOwner && !MATargetRelation::IncludesSelf(TargetRelationMask))
	{
		QueryParams.AddIgnoredActor(SourceActor);
	}

	TArray<FOverlapResult> OverlapResults;
	if (Area.Shape == EMASkillAreaShape::Circle)
	{
		ResolveOverlapResults(World, Area, Area.Circle, ObjectQueryParams, QueryParams, OverlapResults);
	}
	else if (Area.Shape == EMASkillAreaShape::Rect)
	{
		ResolveOverlapResults(World, Area, Area.Rect, ObjectQueryParams, QueryParams, OverlapResults);
	}

	const bool bUseSector = Area.Shape == EMASkillAreaShape::Circle
		&& Area.Circle.bUseSector
		&& Area.Circle.SectorAngle > 0.f
		&& Area.Circle.SectorAngle < 360.f;
	const float SectorCosThreshold = bUseSector
		? FMath::Cos(FMath::DegreesToRadians(Area.Circle.SectorAngle * 0.5f))
		: -1.f;

	TArray<FOverlapResult> FilteredResults;
	TSet<AActor*> SeenActors;
	IGenericTeamAgentInterface* SourceTeam = Cast<IGenericTeamAgentInterface>(SourceActor);
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* HitActor = OverlapResult.GetActor();
		if (!HitActor || SeenActors.Contains(HitActor)) continue;

		if (bUseSector)
		{
			const FVector ToTarget = (HitActor->GetActorLocation() - Area.Center).GetSafeNormal2D();
			if (ToTarget.IsNearlyZero() || FVector::DotProduct(Area.GetForward(), ToTarget) < SectorCosThreshold)
			{
				continue;
			}
		}

		if (SourceTeam)
		{
			if (!MATargetRelation::MatchesTarget(
				TargetRelationMask,
				SourceActor,
				HitActor,
				SourceTeam->GetTeamAttitudeTowards(*HitActor)))
			{
				continue;
			}
		}
		else if (MATargetRelation::IsSelfTarget(SourceActor, HitActor)
			&& !MATargetRelation::IncludesSelf(TargetRelationMask))
		{
			continue;
		}

		SeenActors.Add(HitActor);
		FilteredResults.Add(OverlapResult);
	}

	TArray<FHitResult> HitResults;
	FDebugShapeHelper::ConvertOverlapsToHitResults(FilteredResults, HitResults);
	return HitResults;
}

void MASkillAreaStatics::DrawWorldPreview(
	UWorld& World,
	const FMASkillWorldAreaShape& Area)
{
	if (Area.Shape == EMASkillAreaShape::Circle)
	{
		::DrawWorldPreview(World, Area, Area.Circle);
	}
	else if (Area.Shape == EMASkillAreaShape::Rect)
	{
		::DrawWorldPreview(World, Area, Area.Rect);
	}
	else if (Area.Shape == EMASkillAreaShape::Line)
	{
		::DrawWorldPreview(World, Area, Area.Line);
	}
}
