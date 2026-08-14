#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "MASkillAction_AccumulateModuleStackFromPayload.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Accumulate Module Stack From Payload")
class P_MA_API UMASkillAction_AccumulateModuleStackFromPayload : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_AccumulateModuleStackFromPayload()
	{
		SupportedModuleTypes = EMASkillModuleType::Module;
	}

	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Stack", meta=(Categories="Data"))
	FGameplayTag ValuePayloadTag;

	UPROPERTY(EditDefaultsOnly, Category="Stack", meta=(Categories="Data.Module"))
	FGameplayTag AccumulatedPayloadTag;

	UPROPERTY(EditDefaultsOnly, Category="Stack")
	FMAPayloadCalculation StackCalculation;
};
