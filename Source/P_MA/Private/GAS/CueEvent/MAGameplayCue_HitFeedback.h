#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "MAGameplayCue_HitFeedback.generated.h"

UCLASS()
class P_MA_API UMAGameplayCue_HitFeedback : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual void HandleGameplayCue(
		AActor* MyTarget,
		EGameplayCueEvent::Type EventType,
		const FGameplayCueParameters& Parameters) override;
private:
	void PlayHitSound(
		AActor* MyTarget,
		const FGameplayCueParameters& Parameters) const;
};
