#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "MAGameplaySoundLibrary.generated.h"

class USoundBase;

UCLASS(NotBlueprintable)
class P_MA_API UMAGameplaySoundLibrary : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const FPrimaryAssetType PrimaryAssetType;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	USoundBase* FindSound(FGameplayTag SoundTag) const;

#if WITH_EDITOR
	void GetSoundTagsForEditor(TArray<FGameplayTag>& OutTags) const { Sounds.GetKeys(OutTags); }
	bool AddSoundEntryForEditor(FGameplayTag SoundTag);
	bool RenameSoundEntryForEditor(FGameplayTag SoundTag, FGameplayTag NewSoundTag);
	bool RemoveSoundEntryForEditor(FGameplayTag SoundTag);
	bool SetSoundForEditor(FGameplayTag SoundTag, USoundBase* Sound);
#endif

private:
	UPROPERTY(EditAnywhere, Category = "Sound")
	TMap<FGameplayTag, TObjectPtr<USoundBase>> Sounds;
};
