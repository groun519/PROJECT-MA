#include "GAS/Skill/CrowdControl/MASkillCrowdControl.h"
#include "GameplayEffect.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/CrowdControl/MAGameplayEffect_CrowdControlDuration.h"
#include "GAS/Skill/MASkillAbility.h"

namespace
{
const UGameplayEffect* GetDurationCrowdControlGameplayEffectTemplate()
{
	return GetDefault<UMAGameplayEffect_CrowdControlDuration>();
}

FGameplayEffectSpecHandle MakeCrowdControlGameplayEffectSpec(UMASkillAbility& SkillAbility, const UGameplayEffect* EffectDefinition, float Level)
{
	if (!EffectDefinition) return FGameplayEffectSpecHandle();

	FGameplayEffectSpecHandle SpecHandle(new FGameplayEffectSpec(
		EffectDefinition,
		SkillAbility.MakeEffectContext(SkillAbility.GetCurrentAbilitySpecHandle(), SkillAbility.GetCurrentActorInfo()),
		Level));

	if (SpecHandle.IsValid())
	{
		if (FGameplayAbilitySpec* AbilitySpec = SkillAbility.GetCurrentAbilitySpec())
		{
			SkillAbility.ApplyAbilityTagsToGameplayEffectSpec(*SpecHandle.Data.Get(), AbilitySpec);
			SpecHandle.Data->SetByCallerTagMagnitudes = AbilitySpec->SetByCallerTagMagnitudes;
		}
	}

	return SpecHandle;
}
}

bool UMASkillCrowdControl::BuildResolvedEffect(UMASkillAbility& SkillAbility, TArray<FResolvedCrowdControlEffect>& OutEffects) const
{
	FMASkillCrowdControlPolicy Policy;
	if (!ResolvePolicy(Policy) || !Policy.IsValid()) return false;

	FGameplayEffectSpecHandle SpecHandle = MakeCrowdControlGameplayEffectSpec(SkillAbility, GetDurationCrowdControlGameplayEffectTemplate(), 1.f);
	if (!SpecHandle.IsValid()) return false;

	SpecHandle.Data->DynamicGrantedTags.AppendTags(Policy.GrantedStateTags);
	SpecHandle.Data->DynamicGrantedTags.AddTag(Policy.CrowdControlTag);
	SpecHandle.Data->SetDuration(Policy.Duration, true);

	if (!FMath::IsNearlyZero(Policy.Magnitude))
	{
		SpecHandle.Data->SetSetByCallerMagnitude(Policy.CrowdControlTag, Policy.Magnitude);
	}

	ApplyCustomPayload(SpecHandle);

	FResolvedCrowdControlEffect ResolvedCrowdControlEffect;
	ResolvedCrowdControlEffect.SpecHandle = SpecHandle;
	ResolvedCrowdControlEffect.SourceType = Policy.SourceType;
	OutEffects.Add(ResolvedCrowdControlEffect);
	return true;
}

void UMASkillCrowdControl::AppendGrantedStateTags(const FMASkillCrowdControlGrantedStateRule& Rule, FGameplayTagContainer& GrantedTags)
{
	if (Rule.bBlockMove)
	{
		GrantedTags.AddTag(UMAAbilitySystemStatics::GetMoveBlockTag());
	}

	if (Rule.bLockRotation)
	{
		GrantedTags.AddTag(UMAAbilitySystemStatics::GetRotationLockTag());
	}

	if (Rule.bBlockAbility)
	{
		GrantedTags.AddTag(UMAAbilitySystemStatics::GetAbilityBlockTag());
	}
}
