#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/MASkillEventTypes.h"
#include "GAS/Skill/Module/MASkillModuleTypes.h"
#include "UObject/Object.h"
#include "MASkillAction.generated.h"

class AActor;
class UMASkillAbility;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillAction : public UObject
{
	GENERATED_BODY()

public:
	bool SupportsModuleType(const EMASkillModuleType ModuleType) const
	{
		return EnumHasAnyFlags(SupportedModuleTypes, ModuleType);
	}

	/** Owner is always valid; Ability and Scopes are only provided for skill execution. */
	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes)
		PURE_VIRTUAL(UMASkillAction::Execute, );

protected:
	/** Module types that may execute this action. Action classes must declare their supported types. */
	EMASkillModuleType SupportedModuleTypes = EMASkillModuleType::None;
};
