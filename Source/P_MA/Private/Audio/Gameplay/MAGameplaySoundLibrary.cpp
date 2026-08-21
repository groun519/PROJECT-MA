#include "Audio/Gameplay/MAGameplaySoundLibrary.h"

const FPrimaryAssetType UMAGameplaySoundLibrary::PrimaryAssetType(TEXT("GameplaySoundLibrary"));

FPrimaryAssetId UMAGameplaySoundLibrary::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(PrimaryAssetType, GetFName());
}

const FMASoundLayers* UMAGameplaySoundLibrary::FindLayers(const FGameplayTag SoundTag) const
{
	return SoundLayers.Find(SoundTag);
}
