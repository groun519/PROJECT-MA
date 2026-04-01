#pragma once

#include "CoreMinimal.h"
#include "MASkillRuntimeContext.generated.h"

USTRUCT(BlueprintType)
struct FSkillRuntimeContext
{
	GENERATED_BODY()

	void ClearIgnoredActors()
	{
		IgnoredActors.Reset();
	}

	bool IsIgnoredActor(const AActor* Actor) const
	{
		return Actor && IgnoredActors.Contains(Actor);
	}

	void AddIgnoredActor(AActor* Actor)
	{
		if (Actor)
		{
			IgnoredActors.Add(Actor);
		}
	}

private:
	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> IgnoredActors;
};
