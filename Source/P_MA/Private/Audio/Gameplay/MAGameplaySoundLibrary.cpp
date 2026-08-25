#include "Audio/Gameplay/MAGameplaySoundLibrary.h"

#include "Sound/SoundBase.h"

const FPrimaryAssetType UMAGameplaySoundLibrary::PrimaryAssetType(TEXT("GameplaySoundLibrary"));

FPrimaryAssetId UMAGameplaySoundLibrary::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(PrimaryAssetType, GetFName());
}

USoundBase* UMAGameplaySoundLibrary::FindSound(const FGameplayTag SoundTag) const
{
	const TObjectPtr<USoundBase>* Sound = Sounds.Find(SoundTag);
	return Sound ? Sound->Get() : nullptr;
}

#if WITH_EDITOR
bool UMAGameplaySoundLibrary::AddSoundEntryForEditor(const FGameplayTag SoundTag)
{
	if (!SoundTag.IsValid() || Sounds.Contains(SoundTag)) return false;
	Modify();
	Sounds.Add(SoundTag, nullptr);
	PostEditChange();
	MarkPackageDirty();
	return true;
}

bool UMAGameplaySoundLibrary::RenameSoundEntryForEditor(
	const FGameplayTag SoundTag,
	const FGameplayTag NewSoundTag)
{
	if (SoundTag == NewSoundTag) return true;
	if (!NewSoundTag.IsValid() || Sounds.Contains(NewSoundTag)) return false;
	TObjectPtr<USoundBase>* Sound = Sounds.Find(SoundTag);
	if (!Sound) return false;

	Modify();
	TObjectPtr<USoundBase> MovedSound = *Sound;
	Sounds.Remove(SoundTag);
	Sounds.Add(NewSoundTag, MovedSound);
	PostEditChange();
	MarkPackageDirty();
	return true;
}

bool UMAGameplaySoundLibrary::RemoveSoundEntryForEditor(const FGameplayTag SoundTag)
{
	if (!Sounds.Contains(SoundTag)) return false;
	Modify();
	Sounds.Remove(SoundTag);
	PostEditChange();
	MarkPackageDirty();
	return true;
}

bool UMAGameplaySoundLibrary::SetSoundForEditor(
	const FGameplayTag SoundTag,
	USoundBase* Sound)
{
	TObjectPtr<USoundBase>* MappedSound = Sounds.Find(SoundTag);
	if (!MappedSound) return false;
	if (*MappedSound == Sound) return true;
	Modify();
	*MappedSound = Sound;
	PostEditChange();
	MarkPackageDirty();
	return true;
}
#endif
