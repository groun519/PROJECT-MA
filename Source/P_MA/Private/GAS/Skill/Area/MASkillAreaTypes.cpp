#include "GAS/Skill/Area/MASkillAreaTypes.h"

static constexpr float HitAreaDepth = 110.f;

static void ResolveWorldArea(FMASkillWorldAreaShape& Result, const FMASkillCircleArea& Source)
{
	Result.Circle.Radius = Source.Radius;
	Result.Circle.bUseSector = Source.bUseSector;
	Result.Circle.SectorAngle = Source.SectorAngle;
}

static void ResolveWorldArea(FMASkillWorldAreaShape& Result, const FMASkillRectArea& Source)
{
	Result.Rect.Width = Source.Width;
	Result.Rect.Height = Source.Height;
	Result.Rect.Depth = HitAreaDepth;
}

static void ResolveWorldArea(FMASkillWorldAreaShape& Result, const FMASkillLineArea& Source)
{
	Result.Line.Width = Source.Width;
	Result.Line.Length = Source.Length;
	Result.Center += Result.GetForward() * (Result.Line.Length * 0.5f);
}

bool FMASkillWorldAreaShape::IsValid() const
{
	switch (Shape)
	{
	case EMASkillAreaShape::Circle:
		return Circle.IsValid();
	case EMASkillAreaShape::Rect:
		return Rect.IsValid();
	case EMASkillAreaShape::Line:
		return Line.IsValid();
	default:
		return false;
	}
}

FMASkillWorldAreaShape FMASkillAreaShape::ResolveWorld(const FTransform& OriginTransform) const
{
	FMASkillWorldAreaShape Result;
	Result.Shape = Shape;
	Result.Center = OriginTransform.TransformPosition(FVector(LocalOffset, 0.f));
	Result.Rotation = FRotator(0.f, OriginTransform.Rotator().Yaw + LocalYaw + 90.f, 0.f);
	Result.bIgnoreOwner = bIgnoreOwner;
	Result.bDrawDebug = bDrawDebug;
	Result.DebugColor = DebugColor;
	Result.DebugThickness = DebugThickness;

	switch (Shape)
	{
	case EMASkillAreaShape::Circle:
		ResolveWorldArea(Result, Circle);
		break;
	case EMASkillAreaShape::Rect:
		ResolveWorldArea(Result, Rect);
		break;
	case EMASkillAreaShape::Line:
		ResolveWorldArea(Result, Line);
		break;
	default:
		break;
	}
	return Result;
}
