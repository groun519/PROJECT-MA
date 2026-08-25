#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AnimNotify_MATaggedSound.generated.h"

UCLASS()
class UAnimNotify_MATaggedSound : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

private:
	UPROPERTY(EditAnywhere, Category = "Sound", meta = (Categories = "Sound.Timing"))
	FGameplayTag SoundTag;

	UPROPERTY(EditAnywhere, Category = "Sound")
	FName SocketName = NAME_None;
};
