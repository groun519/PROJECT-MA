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

#if WITH_EDITOR
	void GetMusicTagsForEditor(TArray<FGameplayTag>& OutTags) const { MusicByTag.GetKeys(OutTags); }
	bool AddMusicEntryForEditor(FGameplayTag MusicTag);
	bool RenameMusicEntryForEditor(FGameplayTag MusicTag, FGameplayTag NewMusicTag);
	bool RemoveMusicEntryForEditor(FGameplayTag MusicTag);
	bool SetMusicForEditor(FGameplayTag MusicTag, USoundBase* Music);
#endif

private:
	UPROPERTY(EditAnywhere, Category = "Music")
	TMap<FGameplayTag, TObjectPtr<USoundBase>> MusicByTag;
};
