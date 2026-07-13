#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Addon/MASkillModuleAddon.h"
#include "GAS/Skill/Event/Binding/MASkillEventBinding.h"
#include "MASkillModuleCooldownAddon.generated.h"

struct FMASkillEvent;

/** Disables its source module for a duration after a matching event is evaluated. */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillModuleCooldownAddon : public UMASkillModuleAddon
{
	GENERATED_BODY()

public:
	float GetDurationSeconds() const { return DurationSeconds; }

	virtual void RegisterEventSubscriptions(
		UMASkillEventDispatcher& EventDispatcher,
		UMASkillModuleInstance& ModuleInstance,
		UMASkillModuleInstance& SkillScope) const override;

private:
	void HandleCooldownEvent(
		const FMASkillEvent& Event,
		TWeakObjectPtr<UMASkillModuleInstance> ModuleInstance,
		TWeakObjectPtr<UMASkillModuleInstance> SkillScope) const;

	UPROPERTY(EditDefaultsOnly, Category="Module Cooldown")
	EMASkillEventBindingScope BindingScope = EMASkillEventBindingScope::Skill;

	UPROPERTY(EditDefaultsOnly, Category="Module Cooldown", meta=(Categories="Event"))
	FGameplayTagContainer TriggerEventTags;

	UPROPERTY(EditDefaultsOnly, Category="Module Cooldown", meta=(ClampMin="0.0", UIMin="0.0"))
	float DurationSeconds = 0.f;
};
