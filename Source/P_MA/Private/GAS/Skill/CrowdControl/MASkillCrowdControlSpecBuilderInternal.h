#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GAS/Skill/CrowdControl/MASkillCrowdControlSpecBuilder.h"
#include "GAS/Skill/MASkillDamageConfig.h"

class UMASkillAbility;

struct FMASkillCrowdControlEntry
{
	FGameplayTag CrowdControlTag;
	float Magnitude = 0.f;
	float Duration = 0.f;
	EMASkillCrowdControlSourceType SourceType = EMASkillCrowdControlSourceType::Instigator;

	bool HasValidData() const
	{
		if (!CrowdControlTag.IsValid()) return false;
		return Duration > 0.f;
	}
};

namespace MASkillCrowdControlSpecBuilderInternal
{
	using FCrowdControlSpecBuildHandler = bool(*)(const FInstancedStruct&, UMASkillAbility&, TArray<FResolvedCrowdControlEffect>&);

	bool BuildCrowdControlEntry(
		const FGameplayTag CrowdControlTag,
		const float Magnitude,
		const float Duration,
		const EMASkillCrowdControlSourceType SourceType,
		FMASkillCrowdControlEntry& OutEntry);

	FGameplayEffectSpecHandle MakeResolvedCrowdControlSpec(UMASkillAbility& SkillAbility, const FMASkillCrowdControlEntry& Entry);
	void AddResolvedCrowdControlEffect(
		const FGameplayEffectSpecHandle& SpecHandle,
		const EMASkillCrowdControlSourceType SourceType,
		TArray<FResolvedCrowdControlEffect>& OutEffects);

	bool TryBuildStunCrowdControlSpec(
		const FInstancedStruct& CrowdControlConfig,
		UMASkillAbility& SkillAbility,
		TArray<FResolvedCrowdControlEffect>& OutEffects);

	bool TryBuildAirborneCrowdControlSpec(
		const FInstancedStruct& CrowdControlConfig,
		UMASkillAbility& SkillAbility,
		TArray<FResolvedCrowdControlEffect>& OutEffects);

	bool TryBuildKnockbackCrowdControlSpec(
		const FInstancedStruct& CrowdControlConfig,
		UMASkillAbility& SkillAbility,
		TArray<FResolvedCrowdControlEffect>& OutEffects);

	bool TryBuildGrabCrowdControlSpec(
		const FInstancedStruct& CrowdControlConfig,
		UMASkillAbility& SkillAbility,
		TArray<FResolvedCrowdControlEffect>& OutEffects);

	bool TryBuildStaggerCrowdControlSpec(
		const FInstancedStruct& CrowdControlConfig,
		UMASkillAbility& SkillAbility,
		TArray<FResolvedCrowdControlEffect>& OutEffects);
}
