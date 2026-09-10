#pragma once

#include "CoreMinimal.h"
#include "MASpaceTransitionTypes.generated.h"

class UWorld;

/** Network identity and placement for one destination Space instance. */
USTRUCT()
struct FMASpaceTransitionRequest
{
	GENERATED_BODY()

	UPROPERTY()
	TSoftObjectPtr<UWorld> DestinationMap;

	UPROPERTY()
	FTransform DestinationSlotTransform = FTransform::Identity;

	UPROPERTY()
	FString DestinationInstanceIdentity;

	UPROPERTY()
	int32 GenerationSeed = 0;

	bool IsValid() const
	{
		return !DestinationMap.IsNull() && !DestinationInstanceIdentity.IsEmpty();
	}
};
