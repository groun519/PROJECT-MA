#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MAMusicSubsystem.generated.h"

class UAudioComponent;
class USoundBase;
class USoundClass;
class UMAMusicLibrary;

UCLASS()
class P_MA_API UMAMusicSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Audio|Music")
	void PlayMusic(FGameplayTag MusicTag);

	UFUNCTION(BlueprintCallable, Category = "Audio|Music")
	void StopMusic();

private:
	void PlayResolvedMusic(USoundBase* Music);
	void FadeOutActiveMusic();
	void HandleAudioFinished(UAudioComponent* FinishedComponent);

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> ActiveMusicComponent;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UAudioComponent>> FadingOutMusicComponents;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> CurrentMusic;

	UPROPERTY(Transient)
	TObjectPtr<USoundClass> BgmSoundClass;

	UPROPERTY(Transient)
	TObjectPtr<UMAMusicLibrary> MusicLibrary;

	static constexpr float MusicFadeDuration = 1.0f;
};
