#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Addon/MASkillModuleAddon.h"
#include "MASkillModuleEventSourceAddon.generated.h"

class UMASkillEventSource;
struct FGameplayTag;

/** Declares event routes provided by this module. */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillModuleEventSourceAddon : public UMASkillModuleAddon
{
	GENERATED_BODY()

public:
	UMASkillModuleEventSourceAddon()
	{
		SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub;
	}

	const TArray<TObjectPtr<UMASkillEventSource>>& GetEventSources() const { return EventSources; }
	bool HasEventSource(const FGameplayTag& EventTag) const;

private:
	virtual UMASkillModuleAddon* AssembleInto(
		UObject& ResultOuter,
		UMASkillModuleAddon* ResultAddon,
		EMASkillAddonAssemblyStage Stage,
		const FMASkillScopes& SourceScopes) const override;

	UPROPERTY(EditDefaultsOnly, Instanced, Category="Event Source")
	TArray<TObjectPtr<UMASkillEventSource>> EventSources;
};
