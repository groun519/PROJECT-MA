#include "Animation/Notify/AnimNotify_MATaggedSound.h"

#include "Audio/Gameplay/MAGameplaySoundSubsystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameplayTagsManager.h"

void UAnimNotify_MATaggedSound::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World || World->IsPreviewWorld()) return;

	if (UMAGameplaySoundSubsystem* SoundSubsystem = World->GetSubsystem<UMAGameplaySoundSubsystem>())
	{
		SoundSubsystem->PlayAtLocation(
			SoundTag,
			MeshComp->GetSocketLocation(SocketName),
			MeshComp->GetOwner());
	}
}

FString UAnimNotify_MATaggedSound::GetNotifyName_Implementation() const
{
	if (!SoundTag.IsValid()) return TEXT("Tagged Sound");

	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(SoundTag, TagNames);
	return TagNames.Last().ToString();
}
