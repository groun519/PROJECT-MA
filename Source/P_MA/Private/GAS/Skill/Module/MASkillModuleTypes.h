#pragma once

#include "CoreMinimal.h"
#include "MASkillModuleTypes.generated.h"

class UStaticMesh;

USTRUCT(BlueprintType)
struct P_MA_API FMAStaticMeshVisualData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual")
	TObjectPtr<UStaticMesh> Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual")
	FTransform TransformOffset = FTransform::Identity;
};

UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class EMASkillModuleType : uint8
{
	None   = 0 UMETA(Hidden),
	Module = 1 << 0,
	Item   = 1 << 1,
	Sub    = 1 << 2
};
ENUM_CLASS_FLAGS(EMASkillModuleType);
