#include "Audio/Music/MAMusicSubsystem.h"

#include "Audio/Music/MAMusicLibrary.h"
#include "Audio/Setting/MAAudioSetting.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundClass.h"

DEFINE_LOG_CATEGORY_STATIC(LogMAMusic, Log, All);

bool UMAMusicSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer) && !IsRunningDedicatedServer();
}

void UMAMusicSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (const UMAAudioSetting* AudioSetting = UMAAudioSetting::Get())
	{
		BgmSoundClass = AudioSetting->BgmSoundClass.LoadSynchronous();
		MusicLibrary = AudioSetting->MusicLibrary.LoadSynchronous();
	}

	if (!BgmSoundClass)
	{
		UE_LOG(LogMAMusic, Warning, TEXT("BGM sound class is not configured in MAAudioSetting."));
	}

	if (!MusicLibrary)
	{
		UE_LOG(LogMAMusic, Warning, TEXT("Music library is not configured in MAAudioSetting."));
	}
}

void UMAMusicSubsystem::Deinitialize()
{
	if (ActiveMusicComponent)
	{
		ActiveMusicComponent->OnAudioFinishedNative.RemoveAll(this);
		ActiveMusicComponent->Stop();
	}

	for (UAudioComponent* MusicComponent : FadingOutMusicComponents)
	{
		if (MusicComponent)
		{
			MusicComponent->OnAudioFinishedNative.RemoveAll(this);
			MusicComponent->Stop();
		}
	}

	ActiveMusicComponent = nullptr;
	FadingOutMusicComponents.Reset();
	CurrentMusic = nullptr;
	BgmSoundClass = nullptr;
	MusicLibrary = nullptr;

	Super::Deinitialize();
}

void UMAMusicSubsystem::PlayMusic(const FGameplayTag MusicTag)
{
	if (!ensureMsgf(MusicTag.IsValid(), TEXT("A valid music tag is required.")))
	{
		return;
	}

	if (!MusicLibrary)
	{
		return;
	}

	USoundBase* Music = MusicLibrary->FindMusic(MusicTag);
	if (!Music)
	{
		UE_LOG(LogMAMusic, Warning, TEXT("Music tag '%s' is not registered."), *MusicTag.ToString());
		return;
	}

	PlayResolvedMusic(Music);
}

void UMAMusicSubsystem::PlayResolvedMusic(USoundBase* Music)
{

	if (CurrentMusic == Music && ActiveMusicComponent)
	{
		return;
	}

	UAudioComponent* NewMusicComponent = UGameplayStatics::CreateSound2D(
		this,
		Music,
		1.0f,
		1.0f,
		0.0f,
		nullptr,
		true,
		false);
	if (!NewMusicComponent)
	{
		UE_LOG(LogMAMusic, Warning, TEXT("Failed to create music component for '%s'."), *GetNameSafe(Music));
		return;
	}

	FadeOutActiveMusic();

	CurrentMusic = Music;
	ActiveMusicComponent = NewMusicComponent;
	ActiveMusicComponent->SoundClassOverride = BgmSoundClass;
	ActiveMusicComponent->OnAudioFinishedNative.AddUObject(this, &UMAMusicSubsystem::HandleAudioFinished);
	ActiveMusicComponent->FadeIn(MusicFadeDuration);
}

void UMAMusicSubsystem::StopMusic()
{
	CurrentMusic = nullptr;
	FadeOutActiveMusic();
}

void UMAMusicSubsystem::FadeOutActiveMusic()
{
	if (!ActiveMusicComponent)
	{
		return;
	}

	FadingOutMusicComponents.Add(ActiveMusicComponent);
	ActiveMusicComponent->FadeOut(MusicFadeDuration, 0.0f);
	ActiveMusicComponent = nullptr;
}

void UMAMusicSubsystem::HandleAudioFinished(UAudioComponent* FinishedComponent)
{
	if (!FinishedComponent)
	{
		return;
	}

	if (FinishedComponent == ActiveMusicComponent && CurrentMusic)
	{
		FinishedComponent->Play();
		return;
	}

	FinishedComponent->OnAudioFinishedNative.RemoveAll(this);
	FadingOutMusicComponents.Remove(FinishedComponent);
}
