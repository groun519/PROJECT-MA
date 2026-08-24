#include "Audio/Music/MAMusicLibrary.h"

#include "Sound/SoundBase.h"

const FPrimaryAssetType UMAMusicLibrary::PrimaryAssetType(TEXT("MusicLibrary"));

FPrimaryAssetId UMAMusicLibrary::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(PrimaryAssetType, GetFName());
}

USoundBase* UMAMusicLibrary::FindMusic(const FGameplayTag MusicTag) const
{
	const TObjectPtr<USoundBase>* Music = MusicByTag.Find(MusicTag);
	return Music ? Music->Get() : nullptr;
}
