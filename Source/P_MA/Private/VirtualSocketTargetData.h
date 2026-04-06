#pragma once
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "DebugShapeHelper.h"
#include "VirtualSocketTargetData.generated.h"

USTRUCT(BlueprintType)
struct P_MA_API FGameplayAbilityTargetData_VirtualSocket : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EVA_Shape Shape = EVA_Shape::Sphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LocalOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator LocalRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SphereRadius = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BoxHalfSize = FVector(20.f,12.f,12.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseSector = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SectorAngle = 0.f;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIgnoreOwner = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDrawDebug = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer TriggerGameplayCueTags = FGameplayTagContainer();

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << Shape;
		Ar << LocalOffset;
		Ar << LocalRotation;
		Ar << SphereRadius;
		Ar << BoxHalfSize;
		Ar << bUseSector;
		Ar << SectorAngle;
		Ar << bIgnoreOwner;
		Ar << bDrawDebug;
		for (FGameplayTag CueTag : TriggerGameplayCueTags)
		{
			Ar << CueTag;
		}
		
		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_VirtualSocket>
	: public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_VirtualSocket>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
