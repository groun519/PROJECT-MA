#include "Audio/Gameplay/MAGameplaySoundSubsystem.h"

#include "Audio/Gameplay/MAGameplaySoundLibrary.h"
#include "Audio/Setting/MAAudioSetting.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogMAGameplaySound, Log, All);

bool UMAGameplaySoundSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->GetNetMode() != NM_DedicatedServer;
}

bool UMAGameplaySoundSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UMAGameplaySoundSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UMAAudioSetting* AudioSetting = UMAAudioSetting::Get();
	if (!AudioSetting || AudioSetting->GameplaySoundLibrary.IsNull())
	{
		UE_LOG(LogMAGameplaySound, Warning,
			TEXT("Gameplay sound library is not configured in MAAudioSetting."));
		return;
	}

	GameplaySoundLibrary = AudioSetting->GameplaySoundLibrary.LoadSynchronous();
	if (!GameplaySoundLibrary)
	{
		UE_LOG(LogMAGameplaySound, Warning,
			TEXT("Failed to load gameplay sound library '%s'."),
			*AudioSetting->GameplaySoundLibrary.ToSoftObjectPath().ToString());
	}
}

void UMAGameplaySoundSubsystem::PlayAtLocation(
	const FGameplayTag SoundTag,
	const FVector& Location,
	const AActor* OwningActor)
{
	if (!ensureMsgf(SoundTag.IsValid(), TEXT("A valid sound tag is required.")))
	{
		return;
	}

	if (!GameplaySoundLibrary)
	{
		return;
	}

	const FMASoundLayers* Layers = GameplaySoundLibrary->FindLayers(SoundTag);
	if (!Layers)
	{
		return;
	}

	if (Layers->Sounds.IsEmpty())
	{
		ReportInvalidMappingOnce(SoundTag, TEXT("has no sound layers"));
		return;
	}

	bool bContainsNullSound = false;
	for (USoundBase* Sound : Layers->Sounds)
	{
		if (!Sound)
		{
			bContainsNullSound = true;
			continue;
		}

		UGameplayStatics::PlaySoundAtLocation(
			this,
			Sound,
			Location,
			FRotator::ZeroRotator,
			1.0f,
			1.0f,
			0.0f,
			nullptr,
			nullptr,
			OwningActor);
	}

	if (bContainsNullSound)
	{
		ReportInvalidMappingOnce(SoundTag, TEXT("contains a null sound layer"));
	}
}

void UMAGameplaySoundSubsystem::ReportInvalidMappingOnce(
	const FGameplayTag SoundTag,
	const TCHAR* Reason)
{
	if (ReportedInvalidMappings.Contains(SoundTag))
	{
		return;
	}

	ReportedInvalidMappings.Add(SoundTag);
	UE_LOG(LogMAGameplaySound, Warning,
		TEXT("Gameplay sound mapping '%s' %s."),
		*SoundTag.ToString(),
		Reason);
}
