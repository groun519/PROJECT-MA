#include "GAS/Skill/Damage/MASkillDamageResolver.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/MAElementData.h"
#include "GAS/Skill/Damage/MAGameplayEffect_SkillDamage.h"
#include "GAS/Skill/Damage/MAGameplayEffect_SkillDamageOverTime.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadAccess.h"
#include "GAS/Skill/StatusEffect/MASkillStatusEffect.h"

void MASkillDamageResolver::ApplyDamageOverTimeConfig(
	FGameplayEffectSpecHandle& SpecHandle,
	const FMASkillDamageOverTimeConfig& DamageOverTime)
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
	return ResolveExecutionConfig(
		DamageConfig,
		FMASkillPayloadAccess(nullptr, &PayloadStore, nullptr));
}

FMADamageExecutionConfig MASkillDamageResolver::ResolveExecutionConfig(
	const FMASkillDamageConfig& DamageConfig,
	const FMASkillPayloadAccess& Payloads)
{
	FMADamageExecutionConfig Result;
	Result.BaseDamage = DamageConfig.BaseDamage;
	Result.DamageTypeTag = DamageConfig.DamageTypeTag;

	for (const FMAAttributeCoefficient& Coefficient : DamageConfig.AttributeCoefficients)
	{
		if (FMath::IsNearlyZero(Coefficient.Coefficient)) continue;

		if (Coefficient.Source == EMACoefficientSource::Payload)
		{
			Result.BaseDamage += Coefficient.ResolvePayloadContribution(Payloads);
			continue;
		}
		if (!Coefficient.GameplayAttribute.IsValid()) continue;

		Result.AttributeCoefficients.Add(Coefficient);
	}

	return Result;
}

FMADamageExecutionConfig MASkillDamageResolver::ScaleDamageConfigForTick(const FMADamageExecutionConfig& DamageConfig, int32 TickCount)
{
	FMADamageExecutionConfig Result = DamageConfig;
	const float TickScale = 1.f / static_cast<float>(FMath::Max(TickCount, 1));
	Result.BaseDamage *= TickScale;
	for (FMAAttributeCoefficient& Coefficient : Result.AttributeCoefficients)
	{
		Coefficient.Coefficient *= TickScale;
	}
	return Result;
}

void MASkillDamageResolver::AppendElementalHitGameplayCueTag(
	const FGameplayTag& DamageTypeTag,
	FGameplayTagContainer& TargetGameplayCueTags)
{
	const FMAElementDataRow* ElementRow = FMAElementDataRow::FindByTag(
		DamageTypeTag,
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
	return Resolve(
		OwnerAbility,
		DamageConfig,
		FMASkillPayloadAccess(nullptr, &PayloadStore, nullptr));
}

FResolvedSkillDamage MASkillDamageResolver::Resolve(
	UMASkillAbility& OwnerAbility,
	const FMASkillDamageConfig& DamageConfig,
	const FMASkillPayloadAccess& Payloads)
{
	FResolvedSkillDamage ResolvedDamage;
	ResolvedDamage.ApplicationMode = DamageConfig.ApplicationMode;
	ResolvedDamage.TargetRelationMask = DamageConfig.TargetRelationMask;
	ResolvedDamage.TargetGameplayCueTags = DamageConfig.TargetGameplayCueTags;
	AppendElementalHitGameplayCueTag(DamageConfig.DamageTypeTag, ResolvedDamage.TargetGameplayCueTags);

	const FMADamageExecutionConfig ExecutionConfig = ResolveExecutionConfig(DamageConfig, Payloads);
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

		const float FinalDamageMultiplier = Payloads.Reader.GetScalarProduct(
			UMAAbilitySystemStatics::GetFinalDamageMultiplierTag());
		if (!FMath::IsNearlyEqual(FinalDamageMultiplier, 1.f)
			&& ResolvedDamage.DamageSpec.IsValid()
			&& ResolvedDamage.DamageSpec.Data.IsValid())
		{
			ResolvedDamage.DamageSpec.Data->SetSetByCallerMagnitude(
				UMAAbilitySystemStatics::GetFinalDamageMultiplierTag(),
				FinalDamageMultiplier);
		}

		const float DamageVariance = Payloads.Reader.GetScalarSum(
			UMAAbilitySystemStatics::GetDamageVarianceTag());
		if (!FMath::IsNearlyZero(DamageVariance)
			&& ResolvedDamage.DamageSpec.IsValid()
			&& ResolvedDamage.DamageSpec.Data.IsValid())
		{
			ResolvedDamage.DamageSpec.Data->SetSetByCallerMagnitude(
				UMAAbilitySystemStatics::GetDamageVarianceTag(),
				DamageVariance);
		}

		float FocusOffset = 0.f;
		if (Payloads.Reader.TryGetScalar(
			UMAAbilitySystemStatics::GetSkillFocusOffsetTag(),
			FocusOffset)
			&& !FMath::IsNearlyZero(FocusOffset)
			&& ResolvedDamage.DamageSpec.IsValid()
			&& ResolvedDamage.DamageSpec.Data.IsValid())
		{
			ResolvedDamage.DamageSpec.Data->SetSetByCallerMagnitude(
				UMAAbilitySystemStatics::GetSkillFocusOffsetTag(),
				FocusOffset);
		}

		float CriticalDamageOffset = 0.f;
		if (Payloads.Reader.TryGetScalar(
			UMAAbilitySystemStatics::GetSkillCriticalDamageOffsetTag(),
			CriticalDamageOffset)
			&& !FMath::IsNearlyZero(CriticalDamageOffset)
			&& ResolvedDamage.DamageSpec.IsValid()
			&& ResolvedDamage.DamageSpec.Data.IsValid())
		{
			ResolvedDamage.DamageSpec.Data->SetSetByCallerMagnitude(
				UMAAbilitySystemStatics::GetSkillCriticalDamageOffsetTag(),
				CriticalDamageOffset);
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
