#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/Publish/MASkillEventSource.h"
#include "MASkillEventSource_DamageDealt.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillEventSource_DamageDealt : public UMASkillEventSource
{
	GENERATED_BODY()

public:
	UMASkillEventSource_DamageDealt();

	virtual void HandleSourceEvent(
		UMASkillAbility& SkillAbility,
		UMASkillModuleInstance& InEventOwnerScope,
		const FGameplayTag& SourceEventTag,
		const FGameplayEventData& EventData) const override;
};
