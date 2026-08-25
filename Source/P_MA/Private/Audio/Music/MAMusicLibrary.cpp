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

#if WITH_EDITOR
bool UMAMusicLibrary::AddMusicEntryForEditor(const FGameplayTag MusicTag)
{
	if (!MusicTag.IsValid() || MusicByTag.Contains(MusicTag)) return false;
	Modify();
	MusicByTag.Add(MusicTag);
	PostEditChange();
	MarkPackageDirty();
	return true;
}

bool UMAMusicLibrary::RenameMusicEntryForEditor(
	const FGameplayTag MusicTag,
	const FGameplayTag NewMusicTag)
{
	if (MusicTag == NewMusicTag) return true;
	if (!NewMusicTag.IsValid() || MusicByTag.Contains(NewMusicTag)) return false;
	TObjectPtr<USoundBase>* Music = MusicByTag.Find(MusicTag);
	if (!Music) return false;

	Modify();
	TObjectPtr<USoundBase> MovedMusic = *Music;
	MusicByTag.Remove(MusicTag);
	MusicByTag.Add(NewMusicTag, MovedMusic);
	PostEditChange();
	MarkPackageDirty();
	return true;
}

bool UMAMusicLibrary::RemoveMusicEntryForEditor(const FGameplayTag MusicTag)
{
	if (!MusicByTag.Contains(MusicTag)) return false;
	Modify();
	MusicByTag.Remove(MusicTag);
	PostEditChange();
	MarkPackageDirty();
	return true;
}

bool UMAMusicLibrary::SetMusicForEditor(const FGameplayTag MusicTag, USoundBase* Music)
{
	TObjectPtr<USoundBase>* CurrentMusic = MusicByTag.Find(MusicTag);
	if (!CurrentMusic) return false;
	if (*CurrentMusic == Music) return true;
	Modify();
	*CurrentMusic = Music;
	PostEditChange();
	MarkPackageDirty();
	return true;
}
#endif
