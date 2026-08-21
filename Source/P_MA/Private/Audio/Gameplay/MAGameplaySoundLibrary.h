#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "MAGameplaySoundLibrary.generated.h"

class USoundBase;

USTRUCT()
struct FMASoundLayers
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Sound")
	TArray<TObjectPtr<USoundBase>> Sounds;
};

UCLASS(NotBlueprintable)
class P_MA_API UMAGameplaySoundLibrary : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const FPrimaryAssetType PrimaryAssetType;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	const FMASoundLayers* FindLayers(const FGameplayTag SoundTag) const;

private:
	UPROPERTY(EditAnywhere, Category = "Sound")
	TMap<FGameplayTag, FMASoundLayers> SoundLayers;
};
