// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MAAudioSettingsSubsystem.generated.h"

class USoundClass;
class USoundMix;

UCLASS()
class P_MA_API UMAAudioSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Lifecycle **/
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Audio **/
	UFUNCTION(BlueprintCallable, Category = "Audio")
	float GetCurrentMasterVolume() const;

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetCurrentMasterVolume(float InVolume);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	float GetCurrentBgmVolume() const;

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetCurrentBgmVolume(float InVolume);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	float GetCurrentSfxVolume() const;

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetCurrentSfxVolume(float InVolume);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	float GetCurrentUiVolume() const;

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetCurrentUiVolume(float InVolume);

private:
	/** Lifecycle **/
	void LoadAudioSettings();

	/** Setting **/
	USoundMix* GetSettingsSoundMix() const;
	USoundClass* GetMasterSoundClass() const;
	USoundClass* GetBgmSoundClass() const;
	USoundClass* GetSfxSoundClass() const;
	USoundClass* GetUiSoundClass() const;

	/** Audio **/
	void SaveAudioVolume(const TCHAR* ConfigKey, float InVolume) const;
	float LoadAudioVolume(const TCHAR* ConfigKey, float InDefaultVolume) const;
	void ApplyAudioSettings() const;
	void ApplySoundClassVolume(USoundMix* InSoundMix, USoundClass* InSoundClass, float InVolume, bool bApplyToChildren) const;
	float NormalizeAudioVolume(float InVolume) const;

	/** Assets **/
	mutable TObjectPtr<USoundMix> CachedSettingsSoundMix;
	mutable TObjectPtr<USoundClass> CachedMasterSoundClass;
	mutable TObjectPtr<USoundClass> CachedBgmSoundClass;
	mutable TObjectPtr<USoundClass> CachedSfxSoundClass;
	mutable TObjectPtr<USoundClass> CachedUiSoundClass;

	/** Audio **/
	float CurrentMasterVolume = 1.0f;
	float CurrentBgmVolume = 1.0f;
	float CurrentSfxVolume = 1.0f;
	float CurrentUiVolume = 1.0f;
};
