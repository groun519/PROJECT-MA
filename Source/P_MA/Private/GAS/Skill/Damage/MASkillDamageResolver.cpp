#include "GAS/Skill/Damage/MASkillDamageResolver.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/MAElementData.h"
#include "GAS/Skill/Damage/MAGameplayEffect_SkillDamage.h"
#include "GAS/Skill/Damage/MAGameplayEffect_SkillDamageOverTime.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GAS/Skill/StatusEffect/MASkillStatusEffect.h"

void MASkillDamageResolver::ApplyDamageOverTimeConfig(FGameplayEffectSpecHandle& SpecHandle, const FMASkillDamageOverTimeConfig& DamageOverTime)
{
	if (!SpecHandle.IsValid()) return;

	const float Duration = FMath::Max(DamageOverTime.Duration, 0.01f);
	const int32 TickCount = FMath::Max(DamageOverTime.TickCount, 1);
	SpecHandle.Data->SetDuration(Duration, true);
	SpecHandle.Data->Period = Duration / static_cast<float>(TickCount);
}

FMADamageExecutionConfig MASkillDamageResolver::ResolveExecutionConfig(
	const FMASkillDamageConfig& DamageConfig,
	const FMASkillPayloadStore& PayloadStore)
{
	FMADamageExecutionConfig Result;
	Result.BaseDamage = DamageConfig.BaseDamage;
	Result.DamageTypeTag = DamageConfig.DamageTypeTag;

	for (const FMADamageAttributeCoefficient& Coefficient : DamageConfig.AttributeCoefficients)
	{
		if (FMath::IsNearlyZero(Coefficient.Coefficient)) continue;

		if (Coefficient.Side == EMADamageAttributeSide::Payload)
		{
			float PayloadValue = 0.f;
			if (PayloadStore.TryGetScalar(Coefficient.PayloadTag, PayloadValue))
			{
				Result.BaseDamage += PayloadValue * Coefficient.Coefficient;
			}
			continue;
		}

		Result.AttributeCoefficients.Add(Coefficient);
	}

	return Result;
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

FResolvedSkillDamage MASkillDamageResolver::Resolve(UMASkillAbility& OwnerAbility, const FMASkillDamageConfig& DamageConfig)
{
	return Resolve(OwnerAbility, DamageConfig, OwnerAbility.GetAssembledModulePayloadStore());
}

FResolvedSkillDamage MASkillDamageResolver::Resolve(
	UMASkillAbility& OwnerAbility,
	const FMASkillDamageConfig& DamageConfig,
	const FMASkillPayloadStore& PayloadStore)
{
	FResolvedSkillDamage ResolvedDamage;
	ResolvedDamage.TargetRelationMask = DamageConfig.TargetRelationMask;
	ResolvedDamage.TargetGameplayCueTags = DamageConfig.TargetGameplayCueTags;
	AppendElementalHitGameplayCueTag(OwnerAbility, ResolvedDamage.TargetGameplayCueTags);

	const FMADamageExecutionConfig ExecutionConfig = ResolveExecutionConfig(DamageConfig, PayloadStore);
	if (ExecutionConfig.HasValues())
	{
		const bool bApplyDamageOverTime = DamageConfig.ApplicationMode == EMASkillDamageApplicationMode::DamageOverTime;
		const int32 TickCount = FMath::Max(DamageConfig.DamageOverTime.TickCount, 1);
		const FMADamageExecutionConfig AppliedExecutionConfig = bApplyDamageOverTime
			? ScaleDamageConfigForTick(ExecutionConfig, TickCount)
			: ExecutionConfig;
		ResolvedDamage.DamageSpec = OwnerAbility.MakeDamageEffectSpec(
			bApplyDamageOverTime
				? UMAGameplayEffect_SkillDamageOverTime::StaticClass()
				: UMAGameplayEffect_SkillDamage::StaticClass(),
			1,
			&AppliedExecutionConfig);

		float FinalDamageMultiplier = 1.f;
		if (PayloadStore.TryGetScalar(
			UMAAbilitySystemStatics::GetFinalDamageMultiplierTag(),
			FinalDamageMultiplier)
			&& !FMath::IsNearlyEqual(FinalDamageMultiplier, 1.f)
			&& ResolvedDamage.DamageSpec.IsValid()
			&& ResolvedDamage.DamageSpec.Data.IsValid())
		{
			ResolvedDamage.DamageSpec.Data->SetSetByCallerMagnitude(
				UMAAbilitySystemStatics::GetFinalDamageMultiplierTag(),
				FinalDamageMultiplier);
		}

		float DamageVariance = 0.f;
		if (PayloadStore.TryGetScalar(
			UMAAbilitySystemStatics::GetDamageVarianceTag(),
			DamageVariance)
			&& !FMath::IsNearlyZero(DamageVariance)
			&& ResolvedDamage.DamageSpec.IsValid()
			&& ResolvedDamage.DamageSpec.Data.IsValid())
		{
			ResolvedDamage.DamageSpec.Data->SetSetByCallerMagnitude(
				UMAAbilitySystemStatics::GetDamageVarianceTag(),
				DamageVariance);
		}

		if (bApplyDamageOverTime)
		{
			ApplyDamageOverTimeConfig(ResolvedDamage.DamageSpec, DamageConfig.DamageOverTime);
		}
	}

	for (const TObjectPtr<UMASkillStatusEffect>& StatusEffect : DamageConfig.StatusEffects)
	{
		if (!StatusEffect) continue;
		StatusEffect->BuildResolvedEffect(OwnerAbility, ResolvedDamage.StatusEffects);
	}

	return ResolvedDamage;
}
