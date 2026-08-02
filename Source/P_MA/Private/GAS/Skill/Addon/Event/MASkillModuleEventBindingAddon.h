#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Addon/MASkillModuleAddon.h"
#include "GAS/Skill/Event/Binding/MASkillEventBinding.h"
#include "MASkillModuleEventBindingAddon.generated.h"

/** Declares actions triggered by routed skill events. */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillModuleEventBindingAddon : public UMASkillModuleAddon
{
	GENERATED_BODY()

public:
	UMASkillModuleEventBindingAddon()
	{
		SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub;
	}

	const TArray<FMASkillEventBinding>& GetEventBindings() const { return EventBindings; }

private:
	virtual UMASkillModuleAddon* AssembleInto(
		UObject& ResultOuter,
		UMASkillModuleAddon* ResultAddon,
		EMASkillAddonAssemblyStage Stage,
		const FMASkillScopes& SourceScopes) const override;

	UPROPERTY(EditDefaultsOnly, Category="Event Binding")
	TArray<FMASkillEventBinding> EventBindings;
};
