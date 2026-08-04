#include "GAS/Skill/StatusEffect/MASkillStatusEffect.h"
#include "GameplayEffect.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/StatusEffect/MAGameplayEffect_StatusEffectDuration.h"
#include "GAS/Skill/MASkillAbility.h"

namespace
{
const UGameplayEffect* GetDurationStatusEffectGameplayEffectTemplate()
{
	return GetDefault<UMAGameplayEffect_StatusEffectDuration>();
}
}

FGameplayEffectSpecHandle UMASkillStatusEffect::MakeGameplayEffectSpec(UMASkillAbility& SkillAbility, const UGameplayEffect* EffectDefinition, float Level)
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

bool UMASkillStatusEffect::BuildResolvedEffect(UMASkillAbility& SkillAbility, TArray<FResolvedStatusEffect>& OutEffects) const
{
	FMASkillStatusEffectPolicy Policy;
	if (!ResolvePolicy(Policy) || !Policy.IsValid()) return false;

	FGameplayEffectSpecHandle SpecHandle = MakeGameplayEffectSpec(SkillAbility, GetDurationStatusEffectGameplayEffectTemplate(), 1.f);
	if (!SpecHandle.IsValid()) return false;

	SpecHandle.Data->DynamicGrantedTags.AppendTags(Policy.GrantedStateTags);
	SpecHandle.Data->DynamicGrantedTags.AddTag(Policy.StatusEffectTag);
	SpecHandle.Data->SetDuration(Policy.Duration, true);

	if (!FMath::IsNearlyZero(Policy.Magnitude))
	{
		SpecHandle.Data->SetSetByCallerMagnitude(Policy.StatusEffectTag, Policy.Magnitude);
	}

	ApplyCustomPayload(SpecHandle);

	FResolvedStatusEffect ResolvedStatusEffect;
	ResolvedStatusEffect.SpecHandle = SpecHandle;
	ResolvedStatusEffect.SourceType = Policy.SourceType;
	OutEffects.Add(ResolvedStatusEffect);
	return true;
}

void UMASkillStatusEffect::AppendGrantedStateTags(const FMASkillStatusEffectGrantedStateRule& Rule, FGameplayTagContainer& GrantedTags)
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
