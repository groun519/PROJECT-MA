#include "GAS/Skill/Damage/MASkillDamageResolver.h"

#include "GAS/Skill/MAElementData.h"
#include "GAS/Skill/Damage/MAGameplayEffect_SkillDamage.h"
#include "GAS/Skill/Damage/MAGameplayEffect_SkillDamageOverTime.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/StatusEffect/MASkillStatusEffect.h"

void MASkillDamageResolver::ApplyDamageOverTimeConfig(FGameplayEffectSpecHandle& SpecHandle, const FMASkillDamageOverTimeConfig& DamageOverTime)
{
	if (!SpecHandle.IsValid()) return;

	const float Duration = FMath::Max(DamageOverTime.Duration, 0.01f);
	const int32 TickCount = FMath::Max(DamageOverTime.TickCount, 1);
	SpecHandle.Data->SetDuration(Duration, true);
	SpecHandle.Data->Period = Duration / static_cast<float>(TickCount);
}

FMADamageExecutionConfig MASkillDamageResolver::ScaleDamageConfigForTick(const FMADamageExecutionConfig& DamageConfig, int32 TickCount)
{
	FMADamageExecutionConfig Result = DamageConfig;
	const float TickScale = 1.f / static_cast<float>(FMath::Max(TickCount, 1));
	Result.BaseDamage *= TickScale;
	for (FMADamageAttributeCoefficient& Coefficient : Result.AttributeCoefficients)
	{
		Coefficient.Coefficient *= TickScale;
	}
	return Result;
}

void MASkillDamageResolver::AppendElementalHitGameplayCueTag(UMASkillAbility& OwnerAbility, FGameplayTagContainer& TargetGameplayCueTags)
{
	const FGameplayTag ElementalTag = OwnerAbility.GetElementalTag();
	const UDataTable* ElementalDataTable = OwnerAbility.GetElementalDataTable();
	if (!ElementalDataTable || !ElementalTag.IsValid())
	{
		return;
	}

	FString ElementalRowNameString = ElementalTag.GetTagName().ToString();
	ElementalRowNameString.Split(TEXT("."), nullptr, &ElementalRowNameString, ESearchCase::CaseSensitive, ESearchDir::FromEnd);

	const FMAElementDataRow* ElementRow = ElementalDataTable->FindRow<FMAElementDataRow>(
		FName(*ElementalRowNameString),
		TEXT("SkillDamageElementalHitCueLookup"));
	if (ElementRow && ElementRow->HitGameplayCueTag.IsValid())
	{
		TargetGameplayCueTags.AddTag(ElementRow->HitGameplayCueTag);
	}
}

FResolvedSkillHitEffects MASkillDamageResolver::Resolve(UMASkillAbility& OwnerAbility, const FMASkillDamageConfig& DamageConfig)
{
	FResolvedSkillHitEffects ResolvedHitEffects;
	ResolvedHitEffects.TargetRelationMask = DamageConfig.TargetRelationMask;
	ResolvedHitEffects.TargetGameplayCueTags = DamageConfig.TargetGameplayCueTags;
	AppendElementalHitGameplayCueTag(OwnerAbility, ResolvedHitEffects.TargetGameplayCueTags);

	const FMADamageExecutionConfig ExecutionConfig = DamageConfig.ToExecutionConfig();
	if (ExecutionConfig.HasValues())
	{
		const bool bApplyDamageOverTime = DamageConfig.ApplicationMode == EMASkillDamageApplicationMode::DamageOverTime;
		const int32 TickCount = FMath::Max(DamageConfig.DamageOverTime.TickCount, 1);
		const FMADamageExecutionConfig AppliedExecutionConfig = bApplyDamageOverTime
			? ScaleDamageConfigForTick(ExecutionConfig, TickCount)
			: ExecutionConfig;
		ResolvedHitEffects.DamageSpec = OwnerAbility.MakeDamageEffectSpec(
			bApplyDamageOverTime
				? UMAGameplayEffect_SkillDamageOverTime::StaticClass()
				: UMAGameplayEffect_SkillDamage::StaticClass(),
			1,
			&AppliedExecutionConfig);

		if (bApplyDamageOverTime)
		{
			ApplyDamageOverTimeConfig(ResolvedHitEffects.DamageSpec, DamageConfig.DamageOverTime);
		}
	}

	for (const TObjectPtr<UMASkillStatusEffect>& StatusEffect : DamageConfig.StatusEffects)
	{
		if (!StatusEffect) continue;
		StatusEffect->BuildResolvedEffect(OwnerAbility, ResolvedHitEffects.StatusEffects);
	}

	return ResolvedHitEffects;
}
