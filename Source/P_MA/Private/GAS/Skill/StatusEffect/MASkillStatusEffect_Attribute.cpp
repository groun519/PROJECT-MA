#include "GAS/Skill/StatusEffect/MASkillStatusEffect_Attribute.h"

#include "GAS/Skill/StatusEffect/MAGameplayEffect_StatusEffectAttribute.h"

namespace
{
FGameplayTag GetStatusEffectStrengthMagnitudeTag()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Data.StatusEffect.StrengthMagnitude"));
}
}

bool UMASkillStatusEffect_Attribute::BuildResolvedEffect(UMASkillAbility& SkillAbility, TArray<FResolvedStatusEffect>& OutEffects) const
{
	if (Duration <= 0.f || !EffectTemplate) return false;
	PrepareEffectTemplate();
	const EMASkillStatusEffectStrengthPolicy StrengthPolicy = GetStrengthPolicy();
	const float StrengthMagnitude = GetStrengthMagnitude();

	FGameplayEffectSpecHandle SpecHandle = MakeGameplayEffectSpec(SkillAbility, EffectTemplate, 1.f);
	if (!SpecHandle.IsValid()) return false;

	SpecHandle.Data->DynamicGrantedTags.AppendTags(GrantedTags);
	SpecHandle.Data->SetDuration(Duration, true);
	if (StrengthPolicy != EMASkillStatusEffectStrengthPolicy::None)
	{
		SpecHandle.Data->SetSetByCallerMagnitude(GetStatusEffectStrengthMagnitudeTag(), StrengthMagnitude);
	}
	ApplyCustomPayload(SpecHandle);

	FResolvedStatusEffect ResolvedEffect;
	ResolvedEffect.SpecHandle = SpecHandle;
	ResolvedEffect.StrengthPolicy = StrengthPolicy;
	ResolvedEffect.StrengthMagnitude = StrengthMagnitude;
	OutEffects.Add(ResolvedEffect);
	return true;
}

bool UMASkillStatusEffect_Attribute::ResolvePolicy(FMASkillStatusEffectPolicy& OutPolicy) const
{
	(void)OutPolicy;
	return false;
}

UMASkillStatusEffect_Slow::UMASkillStatusEffect_Slow(const FObjectInitializer& ObjectInitializer)
{
	EffectTemplate = ObjectInitializer.CreateDefaultSubobject<UMAGameplayEffect_StatusEffectSlow>(this, TEXT("EffectTemplate"));
}

void UMASkillStatusEffect_Slow::PrepareEffectTemplate() const
{
	if (EffectTemplate) const_cast<UMAGameplayEffect_StatusEffectAttribute*>(EffectTemplate.Get())->SetMagnitude(Magnitude);
}

UMASkillStatusEffect_Haste::UMASkillStatusEffect_Haste(const FObjectInitializer& ObjectInitializer)
{
	EffectTemplate = ObjectInitializer.CreateDefaultSubobject<UMAGameplayEffect_StatusEffectHaste>(this, TEXT("EffectTemplate"));
}

void UMASkillStatusEffect_Haste::PrepareEffectTemplate() const
{
	if (EffectTemplate) const_cast<UMAGameplayEffect_StatusEffectAttribute*>(EffectTemplate.Get())->SetMagnitude(Magnitude);
}
