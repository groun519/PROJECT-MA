#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimMontage.h"
#include "GameplayTagContainer.h"
#include "MAReactionTypes.generated.h"

UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class EReactionBlockFlags : uint8
{
	None = 0,
	Move = 1 << 0,
	Rotation = 1 << 1,
	Ability = 1 << 2
};
ENUM_CLASS_FLAGS(EReactionBlockFlags);

UENUM(BlueprintType)
enum class EReactionImpulseMode : uint8
{
	None,
	PushFromSource,
	PullToSource
};

USTRUCT()
struct FReactionAnimConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditDefaultsOnly)
	float VerticalLaunchScale = 0.f;
};

USTRUCT(BlueprintType)
struct FReactionRule
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, meta=(Categories="State,Effect"))
	FGameplayTag CrowdControlTag;

	UPROPERTY(EditDefaultsOnly, meta=(Bitmask, BitmaskEnum="/Script/P_MA.EReactionBlockFlags"))
	int32 BlockFlags = static_cast<int32>(EReactionBlockFlags::None);

	UPROPERTY(EditDefaultsOnly)
	EReactionImpulseMode ImpulseMode = EReactionImpulseMode::None;

	UPROPERTY(EditDefaultsOnly)
	bool bPlayMontageOnStart = false;

	UPROPERTY(EditDefaultsOnly)
	bool bStopMontageOnEnd = false;

	UPROPERTY(EditDefaultsOnly)
	bool bStopAIOnStart = false;

	UPROPERTY(EditDefaultsOnly)
	bool bStopMovementOnStart = false;

	bool IsValid() const
	{
		return CrowdControlTag.IsValid();
	}
};
