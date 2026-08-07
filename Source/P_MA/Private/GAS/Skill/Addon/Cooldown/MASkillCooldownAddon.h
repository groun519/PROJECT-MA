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

	float GetCooldownSeconds() const
	{
		return CooldownSeconds * CooldownMultiplier + CooldownOffsetSeconds;
	}

private:
	virtual UMASkillModuleAddon* AssembleInto(
		UObject& ResultOuter,
		UMASkillModuleAddon* ResultAddon,
		EMASkillAddonAssemblyStage Stage,
		const FMASkillScopes& SourceScopes) const override;
	virtual bool Finalize(EMASkillAddonAssemblyStage Stage) override;

	UPROPERTY(EditDefaultsOnly, Category="Cooldown", meta=(ClampMin="0.0", UIMin="0.0"))
	float CooldownSeconds = 0.f;

	/** Module only. Sub modules should use CooldownOffsetSeconds. */
	UPROPERTY(EditDefaultsOnly, Category="Cooldown", meta=(ClampMin="0.0", UIMin="0.0"))
	float CooldownMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Cooldown")
	float CooldownOffsetSeconds = 0.f;
};
