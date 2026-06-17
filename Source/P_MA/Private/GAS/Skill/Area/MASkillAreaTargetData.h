#pragma once

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GAS/Skill/Area/MASkillAreaTypes.h"
#include "MASkillAreaTargetData.generated.h"

USTRUCT(BlueprintType)
struct P_MA_API FGameplayAbilityTargetData_SkillArea : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	UPROPERTY()
	FMASkillWorldAreaShape Area;

	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }

	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << Area.Shape;
		Ar << Area.Center;
		Ar << Area.Rotation;
		switch (Area.Shape)
		{
		case EMASkillAreaShape::Circle:
			Ar << Area.Circle.Radius;
			Ar << Area.Circle.bUseSector;
			Ar << Area.Circle.SectorAngle;
			break;
		case EMASkillAreaShape::Rect:
			Ar << Area.Rect.Width;
			Ar << Area.Rect.Height;
			Ar << Area.Rect.Depth;
			break;
		case EMASkillAreaShape::Line:
			Ar << Area.Line.Width;
			Ar << Area.Line.Length;
			break;
		default:
			break;
		}
		Ar << Area.bIgnoreOwner;
		Ar << Area.bDrawDebug;
		if (Area.bDrawDebug)
		{
			Ar << Area.DebugColor;
			Ar << Area.DebugThickness;
		}

		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_SkillArea>
	: public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_SkillArea>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
