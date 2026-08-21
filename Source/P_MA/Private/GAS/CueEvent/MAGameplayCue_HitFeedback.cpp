#include "GAS/CueEvent/MAGameplayCue_HitFeedback.h"

#include "Audio/Gameplay/MAGameplaySoundSubsystem.h"
#include "Engine/World.h"

void UMAGameplayCue_HitFeedback::HandleGameplayCue(
	AActor* MyTarget,
	const EGameplayCueEvent::Type EventType,
	const FGameplayCueParameters& Parameters)
{
	if (EventType == EGameplayCueEvent::Executed)
	{
		static const FGameplayTag HitRootTag = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Hit"));
		if (Parameters.MatchedTagName == HitRootTag)
		{
			PlayHitSound(MyTarget, Parameters);
		}

		if (Parameters.OriginalTag.IsValid()
			&& Parameters.MatchedTagName != Parameters.OriginalTag)
		{
			return;
		}
	}

	Super::HandleGameplayCue(MyTarget, EventType, Parameters);
}

void UMAGameplayCue_HitFeedback::PlayHitSound(
	AActor* MyTarget,
	const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget)
	{
		return;
	}

	UWorld* World = MyTarget->GetWorld();
	if (!World)
	{
		return;
	}

	UMAGameplaySoundSubsystem* SoundSubsystem = World->GetSubsystem<UMAGameplaySoundSubsystem>();
	if (!SoundSubsystem)
	{
		return;
	}

	const FGameplayTag SoundTag = Parameters.OriginalTag.IsValid()
		? Parameters.OriginalTag
		: Parameters.MatchedTagName;
	const AActor* OwningActor = Parameters.EffectCauser.Get();
	if (!OwningActor)
	{
		OwningActor = Parameters.Instigator.Get();
	}

	SoundSubsystem->PlayAtLocation(SoundTag, Parameters.Location, OwningActor);
}
