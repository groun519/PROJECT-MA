#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "MASkillAction_EmitEventAtModuleStack.generated.h"

/**
 * Converts a module stack value into a semantic event.
 * TODO: Replace this adapter when event bindings can evaluate a module-runtime condition once
 * and fan out the resulting semantic event without duplicating that condition across consumers.
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Emit Event At Module Stack")
class P_MA_API UMASkillAction_EmitEventAtModuleStack : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_EmitEventAtModuleStack()
	{
		SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub;
	}

	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Stack", meta=(ClampMin="0", UIMin="0"))
	int32 RequiredStack = 1;

	UPROPERTY(EditDefaultsOnly, Category="Event", meta=(Categories="Event"))
	FGameplayTag EventTag;
};
