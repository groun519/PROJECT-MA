#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Addon/MASkillModuleAddon.h"
#include "MASkillCooldownAddon.generated.h"

/** Contributes cooldown time to the assembled skill. */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillCooldownAddon : public UMASkillModuleAddon
{
	GENERATED_BODY()

public:
	UMASkillCooldownAddon()
	{
		SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub;
	}

	float GetCooldownSeconds() const { return CooldownSeconds; }

private:
	virtual UMASkillModuleAddon* AssembleInto(
		UObject& ResultOuter,
		UMASkillModuleAddon* ResultAddon,
		EMASkillAddonAssemblyStage Stage,
		const FMASkillScopes& SourceScopes) const override;

	UPROPERTY(EditDefaultsOnly, Category="Cooldown", meta=(ClampMin="0.0", UIMin="0.0"))
	float CooldownSeconds = 0.f;
};
