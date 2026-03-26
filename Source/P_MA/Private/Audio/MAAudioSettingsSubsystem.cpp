// Fill out your copyright notice in the Description page of Project Settings.

#include "Audio/MAAudioSettingsSubsystem.h"

#include "Audio/Setting/MAAudioSetting.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"

namespace
{
	const TCHAR* GAudioSettingsSection = TEXT("MA.Audio");
	const TCHAR* GMasterVolumeKey = TEXT("MasterVolume");
	const TCHAR* GBgmVolumeKey = TEXT("BgmVolume");
	const TCHAR* GSfxVolumeKey = TEXT("SfxVolume");
	const TCHAR* GUiVolumeKey = TEXT("UiVolume");
}

/** Lifecycle **/
void UMAAudioSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadAudioSettings();
}

void UMAAudioSettingsSubsystem::LoadAudioSettings()
{
	const UMAAudioSetting* AudioSetting = UMAAudioSetting::Get();
	CurrentMasterVolume = LoadAudioVolume(GMasterVolumeKey, AudioSetting ? AudioSetting->DefaultMasterVolume : 1.0f);
	CurrentBgmVolume = LoadAudioVolume(GBgmVolumeKey, AudioSetting ? AudioSetting->DefaultBgmVolume : 1.0f);
	CurrentSfxVolume = LoadAudioVolume(GSfxVolumeKey, AudioSetting ? AudioSetting->DefaultSfxVolume : 1.0f);
	CurrentUiVolume = LoadAudioVolume(GUiVolumeKey, AudioSetting ? AudioSetting->DefaultUiVolume : 1.0f);
	ApplyAudioSettings();
}

/** Setting **/
USoundMix* UMAAudioSettingsSubsystem::GetSettingsSoundMix() const
{
	if (!CachedSettingsSoundMix)
	{
		if (const UMAAudioSetting* AudioSetting = UMAAudioSetting::Get())
		{
			CachedSettingsSoundMix = AudioSetting->SettingsSoundMix.LoadSynchronous();
		}
	}

	return CachedSettingsSoundMix;
}

USoundClass* UMAAudioSettingsSubsystem::GetMasterSoundClass() const
{
	if (!CachedMasterSoundClass)
	{
		if (const UMAAudioSetting* AudioSetting = UMAAudioSetting::Get())
		{
			CachedMasterSoundClass = AudioSetting->MasterSoundClass.LoadSynchronous();
		}
	}

	return CachedMasterSoundClass;
}

USoundClass* UMAAudioSettingsSubsystem::GetBgmSoundClass() const
{
	if (!CachedBgmSoundClass)
	{
		if (const UMAAudioSetting* AudioSetting = UMAAudioSetting::Get())
		{
			CachedBgmSoundClass = AudioSetting->BgmSoundClass.LoadSynchronous();
		}
	}

	return CachedBgmSoundClass;
}

USoundClass* UMAAudioSettingsSubsystem::GetSfxSoundClass() const
{
	if (!CachedSfxSoundClass)
	{
		if (const UMAAudioSetting* AudioSetting = UMAAudioSetting::Get())
		{
			CachedSfxSoundClass = AudioSetting->SfxSoundClass.LoadSynchronous();
		}
	}

	return CachedSfxSoundClass;
}

USoundClass* UMAAudioSettingsSubsystem::GetUiSoundClass() const
{
	if (!CachedUiSoundClass)
	{
		if (const UMAAudioSetting* AudioSetting = UMAAudioSetting::Get())
		{
			CachedUiSoundClass = AudioSetting->UiSoundClass.LoadSynchronous();
		}
	}

	return CachedUiSoundClass;
}

/** Audio **/
float UMAAudioSettingsSubsystem::GetCurrentMasterVolume() const
{
	return CurrentMasterVolume;
}

void UMAAudioSettingsSubsystem::SetCurrentMasterVolume(float InVolume)
{
	CurrentMasterVolume = NormalizeAudioVolume(InVolume);
	ApplyAudioSettings();
	SaveAudioVolume(GMasterVolumeKey, CurrentMasterVolume);
}

float UMAAudioSettingsSubsystem::GetCurrentBgmVolume() const
{
	return CurrentBgmVolume;
}

void UMAAudioSettingsSubsystem::SetCurrentBgmVolume(float InVolume)
{
	CurrentBgmVolume = NormalizeAudioVolume(InVolume);
	ApplyAudioSettings();
	SaveAudioVolume(GBgmVolumeKey, CurrentBgmVolume);
}

float UMAAudioSettingsSubsystem::GetCurrentSfxVolume() const
{
	return CurrentSfxVolume;
}

void UMAAudioSettingsSubsystem::SetCurrentSfxVolume(float InVolume)
{
	CurrentSfxVolume = NormalizeAudioVolume(InVolume);
	ApplyAudioSettings();
	SaveAudioVolume(GSfxVolumeKey, CurrentSfxVolume);
}

float UMAAudioSettingsSubsystem::GetCurrentUiVolume() const
{
	return CurrentUiVolume;
}

void UMAAudioSettingsSubsystem::SetCurrentUiVolume(float InVolume)
{
	CurrentUiVolume = NormalizeAudioVolume(InVolume);
	ApplyAudioSettings();
	SaveAudioVolume(GUiVolumeKey, CurrentUiVolume);
}

void UMAAudioSettingsSubsystem::SaveAudioVolume(const TCHAR* ConfigKey, float InVolume) const
{
	GConfig->SetFloat(GAudioSettingsSection, ConfigKey, NormalizeAudioVolume(InVolume), GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
}

float UMAAudioSettingsSubsystem::LoadAudioVolume(const TCHAR* ConfigKey, float InDefaultVolume) const
{
	float Volume = InDefaultVolume;
	GConfig->GetFloat(GAudioSettingsSection, ConfigKey, Volume, GGameUserSettingsIni);
	return NormalizeAudioVolume(Volume);
}

void UMAAudioSettingsSubsystem::ApplyAudioSettings() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USoundMix* SettingsSoundMix = GetSettingsSoundMix())
		{
			UGameplayStatics::SetBaseSoundMix(GameInstance, SettingsSoundMix);
			ApplySoundClassVolume(SettingsSoundMix, GetMasterSoundClass(), CurrentMasterVolume, true);
			ApplySoundClassVolume(SettingsSoundMix, GetBgmSoundClass(), CurrentBgmVolume, false);
			ApplySoundClassVolume(SettingsSoundMix, GetSfxSoundClass(), CurrentSfxVolume, false);
			ApplySoundClassVolume(SettingsSoundMix, GetUiSoundClass(), CurrentUiVolume, false);
		}
	}
}

void UMAAudioSettingsSubsystem::ApplySoundClassVolume(USoundMix* InSoundMix, USoundClass* InSoundClass, float InVolume, bool bApplyToChildren) const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (InSoundMix)
		{
			if (InSoundClass)
			{
				UGameplayStatics::SetSoundMixClassOverride(
					GameInstance,
					InSoundMix,
					InSoundClass,
					NormalizeAudioVolume(InVolume),
					1.0f,
					0.0f,
					bApplyToChildren
				);
			}
		}
	}
}

float UMAAudioSettingsSubsystem::NormalizeAudioVolume(float InVolume) const
{
	return FMath::Clamp(InVolume, 0.0f, 1.0f);
}
