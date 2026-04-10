#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimMontage.h"
#include "GameplayTagContainer.h"
#include "MAStatusEffectTypes.generated.h"

UENUM(BlueprintType)
enum class EStatusEffectImpulseMode : uint8
{
	None,
	PushFromSource,
	PullToSource
};

USTRUCT()
struct FStatusEffectAnimConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditDefaultsOnly)
	float VerticalLaunchScale = 0.f;
};

USTRUCT(BlueprintType)
struct FStatusEffectRule
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, meta=(Categories="State,Effect"))
	FGameplayTag CrowdControlTag;

	UPROPERTY(EditDefaultsOnly)
	EStatusEffectImpulseMode ImpulseMode = EStatusEffectImpulseMode::None;

	UPROPERTY(EditDefaultsOnly)
	bool bPlayMontageOnStart = false;

	UPROPERTY(EditDefaultsOnly)
	bool bStopMontageOnEnd = false;

	UPROPERTY(EditDefaultsOnly)
	bool bStopMovementOnStart = false;

	bool IsValid() const
	{
		return CrowdControlTag.IsValid();
	}

	bool HasImpulseEffect() const
	{
		return ImpulseMode != EStatusEffectImpulseMode::None;
	}
};
