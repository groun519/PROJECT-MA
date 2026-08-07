#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "MASkillAction_ModifyModuleStack.generated.h"

UENUM(BlueprintType)
enum class EMASkillModuleStackOperation : uint8
{
	Add,
	Set,
	Clear
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Modify Module Stack")
class P_MA_API UMASkillAction_ModifyModuleStack : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_ModifyModuleStack() { SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub; }

	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Stack")
	EMASkillModuleStackOperation Operation = EMASkillModuleStackOperation::Add;

	UPROPERTY(EditDefaultsOnly, Category="Stack", meta=(EditCondition="Operation != EMASkillModuleStackOperation::Clear", EditConditionHides))
	int32 Value = 1;

	/** Replaces Value with the rounded payload value when set. */
	UPROPERTY(EditDefaultsOnly, Category="Stack", meta=(Categories="Data", EditCondition="Operation != EMASkillModuleStackOperation::Clear", EditConditionHides))
	FGameplayTag ValuePayloadTag;
};
