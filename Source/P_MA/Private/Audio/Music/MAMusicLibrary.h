#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "MAMusicLibrary.generated.h"

class USoundBase;

UCLASS(NotBlueprintable)
class P_MA_API UMAMusicLibrary : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const FPrimaryAssetType PrimaryAssetType;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	USoundBase* FindMusic(FGameplayTag MusicTag) const;

private:
	UPROPERTY(EditAnywhere, Category = "Music")
	TMap<FGameplayTag, TObjectPtr<USoundBase>> MusicByTag;
};
