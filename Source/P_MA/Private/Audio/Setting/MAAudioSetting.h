// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MAAudioSetting.generated.h"

class USoundClass;
class USoundMix;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Audio Setting"))
class P_MA_API UMAAudioSetting : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static const UMAAudioSetting* Get() { return GetDefault<UMAAudioSetting>(); }

	/** Assets **/
	UPROPERTY(Config, EditAnywhere, Category = "Assets")
	TSoftObjectPtr<USoundMix> SettingsSoundMix;

	UPROPERTY(Config, EditAnywhere, Category = "Assets")
	TSoftObjectPtr<USoundClass> MasterSoundClass;

	UPROPERTY(Config, EditAnywhere, Category = "Assets")
	TSoftObjectPtr<USoundClass> BgmSoundClass;

	UPROPERTY(Config, EditAnywhere, Category = "Assets")
	TSoftObjectPtr<USoundClass> SfxSoundClass;

	UPROPERTY(Config, EditAnywhere, Category = "Assets")
	TSoftObjectPtr<USoundClass> UiSoundClass;

	/** Defaults **/
	UPROPERTY(Config, EditAnywhere, Category = "Defaults")
	float DefaultMasterVolume = 1.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Defaults")
	float DefaultBgmVolume = 1.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Defaults")
	float DefaultSfxVolume = 1.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Defaults")
	float DefaultUiVolume = 1.0f;
};
