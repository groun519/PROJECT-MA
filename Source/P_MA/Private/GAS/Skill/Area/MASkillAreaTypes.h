#pragma once

#include "CoreMinimal.h"
#include "MASkillAreaTypes.generated.h"

UENUM(BlueprintType, meta=(ScriptName="MASkillAreaShapeType"))
enum class EMASkillAreaShape : uint8
{
	None,
	Line,
	Circle,
	Rect,
};

USTRUCT(BlueprintType)
struct P_MA_API FMASkillCircleArea
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Circle", meta=(ClampMin="0.0"))
	float Radius = 100.f;

	UPROPERTY(EditAnywhere, Category="Circle")
	bool bUseSector = false;

	UPROPERTY(EditAnywhere, Category="Circle", meta=(EditCondition="bUseSector", EditConditionHides, ClampMin="0.0", ClampMax="360.0"))
	float SectorAngle = 90.f;
};

USTRUCT(BlueprintType)
struct P_MA_API FMASkillRectArea
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Rect", meta=(ClampMin="0.0"))
	float Width = 200.f;

	UPROPERTY(EditAnywhere, Category="Rect", meta=(ClampMin="0.0"))
	float Height = 100.f;
};

USTRUCT(BlueprintType)
struct P_MA_API FMASkillLineArea
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Line", meta=(ClampMin="0.0"))
	float Width = 100.f;

	UPROPERTY(EditAnywhere, Category="Line", meta=(ClampMin="0.0"))
	float Length = 1000.f;
};

USTRUCT(BlueprintType)
struct P_MA_API FMASkillWorldCircleArea
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	float Radius = 0.f;

	UPROPERTY(Transient)
	bool bUseSector = false;

	UPROPERTY(Transient)
	float SectorAngle = 0.f;

	bool IsValid() const { return Radius > 0.f; }
};

USTRUCT(BlueprintType)
struct P_MA_API FMASkillWorldRectArea
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	float Width = 0.f;

	UPROPERTY(Transient)
	float Height = 0.f;

	UPROPERTY(Transient)
	float Depth = 0.f;

	bool IsValid() const { return Width > 0.f && Height > 0.f && Depth > 0.f; }
};

USTRUCT(BlueprintType)
struct P_MA_API FMASkillWorldLineArea
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	float Width = 0.f;

	UPROPERTY(Transient)
	float Length = 0.f;

	bool IsValid() const { return Width > 0.f && Length > 0.f; }
};

USTRUCT(BlueprintType)
struct P_MA_API FMASkillWorldAreaShape
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	EMASkillAreaShape Shape = EMASkillAreaShape::None;

	UPROPERTY(Transient)
	FVector Center = FVector::ZeroVector;

	UPROPERTY(Transient)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(Transient)
	FMASkillWorldCircleArea Circle;

	UPROPERTY(Transient)
	FMASkillWorldRectArea Rect;

	UPROPERTY(Transient)
	FMASkillWorldLineArea Line;

	UPROPERTY(Transient)
	bool bIgnoreOwner = true;

	UPROPERTY(Transient)
	bool bDrawDebug = false;

	UPROPERTY(Transient)
	FColor DebugColor = FColor::White;

	UPROPERTY(Transient)
	float DebugThickness = 1.f;

	bool IsValid() const;
	bool CanResolveHit() const { return Shape == EMASkillAreaShape::Circle || Shape == EMASkillAreaShape::Rect; }
	FVector GetForward() const { return Rotation.Vector().GetSafeNormal2D(); }
	FRotator GetDecalRotation() const { return FRotator(-90.f, Rotation.Yaw, 0.f); }
};

USTRUCT(BlueprintType)
struct P_MA_API FMASkillAreaShape
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Shape")
	EMASkillAreaShape Shape = EMASkillAreaShape::None;

	UPROPERTY(EditAnywhere, Category="Shape", meta=(EditCondition="Shape == EMASkillAreaShape::Circle", EditConditionHides, ShowOnlyInnerProperties))
	FMASkillCircleArea Circle;

	UPROPERTY(EditAnywhere, Category="Shape", meta=(EditCondition="Shape == EMASkillAreaShape::Rect", EditConditionHides, ShowOnlyInnerProperties))
	FMASkillRectArea Rect;

	UPROPERTY(EditAnywhere, Category="Shape", meta=(EditCondition="Shape == EMASkillAreaShape::Line", EditConditionHides, ShowOnlyInnerProperties))
	FMASkillLineArea Line;

	UPROPERTY(EditAnywhere, Category="Shape")
	FVector2D LocalOffset = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, Category="Shape")
	bool bScaleLocalOffset = false;

	UPROPERTY(EditAnywhere, Category="Shape")
	float LocalYaw = 0.f;

	UPROPERTY(EditAnywhere, Category="Targeting")
	bool bIgnoreOwner = true;

	UPROPERTY(EditAnywhere, Category="Debug")
	bool bDrawDebug = false;

	UPROPERTY(EditAnywhere, Category="Debug", meta=(EditCondition="bDrawDebug", EditConditionHides))
	FColor DebugColor = FColor::White;

	UPROPERTY(EditAnywhere, Category="Debug", meta=(EditCondition="bDrawDebug", EditConditionHides, ClampMin="0.1"))
	float DebugThickness = 1.f;

	FMASkillWorldAreaShape ResolveWorld(const FTransform& OriginTransform, float AreaScale = 1.f) const;
};
