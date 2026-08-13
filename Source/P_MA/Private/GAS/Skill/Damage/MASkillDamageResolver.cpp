#include "GAS/Skill/Damage/MASkillDamageResolver.h"

#include "AbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/MAElementData.h"
#include "GAS/Skill/Damage/MAGameplayEffect_SkillDamage.h"
#include "GAS/Skill/Damage/MAGameplayEffect_SkillDamageOverTime.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Event/MASkillEventTypes.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
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

FMAResolvedDamage MASkillDamageResolver::Resolve(
	UMASkillAbility& OwnerAbility,
	const FMASkillScopes& Scopes,
	const FMASkillDamageConfig& DamageConfig,
	const FMASkillPayloadStore& PayloadStore)
{
	return Resolve(
		OwnerAbility,
		Scopes,
		DamageConfig,
		FMASkillPayloadAccess(nullptr, &PayloadStore, nullptr));
}

FMAResolvedDamage MASkillDamageResolver::Resolve(
	UMASkillAbility& OwnerAbility,
	const FMASkillScopes& Scopes,
	const FMASkillDamageConfig& DamageConfig,
	const FMASkillPayloadAccess& Payloads)
{
	UAbilitySystemComponent* SourceASC = OwnerAbility.GetAbilitySystemComponentFromActorInfo();
	return SourceASC
		? Resolve(
			*SourceASC,
			&OwnerAbility,
			MakeSkillEffectContext(OwnerAbility, Scopes),
			DamageConfig,
			Payloads)
		: FMAResolvedDamage();
}

FMAResolvedDamage MASkillDamageResolver::Resolve(
	UAbilitySystemComponent& SourceASC,
	const FGameplayEffectContextHandle& SourceContext,
	const FMASkillDamageConfig& DamageConfig)
{
	const FMASkillPayloadStore EmptyPayloads;
	return Resolve(
		SourceASC,
		nullptr,
		SourceContext.IsValid() ? SourceContext.Duplicate() : SourceASC.MakeEffectContext(),
		DamageConfig,
		FMASkillPayloadAccess(nullptr, &EmptyPayloads, nullptr));
}

FGameplayEffectContextHandle MASkillDamageResolver::MakeSkillEffectContext(
	UMASkillAbility& OwnerAbility,
	const FMASkillScopes& Scopes)
{
	FGameplayEffectContextHandle Context = OwnerAbility.MakeEffectContext(
		OwnerAbility.GetCurrentAbilitySpecHandle(),
		OwnerAbility.GetCurrentActorInfo());
	Context.AddSourceObject(Scopes.Module.Get());
	if (FMAGameplayEffectContext* MAContext = static_cast<FMAGameplayEffectContext*>(Context.Get()))
	{
		// Keep the original assembled skill instead of reconstructing it after a delayed hit.
		MAContext->SetSkillScope(Scopes.Skill.Get());
	}
	return Context;
}

void MASkillDamageResolver::SetSpecContext(
	FGameplayEffectSpecHandle& SpecHandle,
	const FGameplayEffectContextHandle& SourceContext)
{
	if (SpecHandle.IsValid() && SpecHandle.Data.IsValid())
	{
		SpecHandle.Data->SetContext(SourceContext.Duplicate());
	}
}

FGameplayEffectSpecHandle MASkillDamageResolver::MakeDamageEffectSpec(
	UAbilitySystemComponent& SourceASC,
	UMASkillAbility* OwnerAbility,
	const FGameplayEffectContextHandle& SourceContext,
	bool bDamageOverTime,
	const FMADamageExecutionConfig& DamageConfig)
{
	const TSubclassOf<UGameplayEffect> EffectClass = bDamageOverTime
		? UMAGameplayEffect_SkillDamageOverTime::StaticClass()
		: UMAGameplayEffect_SkillDamage::StaticClass();
	FGameplayEffectSpecHandle SpecHandle = OwnerAbility
		? OwnerAbility->MakeOutgoingGameplayEffectSpec(EffectClass, 1.f)
		: SourceASC.MakeOutgoingSpec(EffectClass, 1.f, SourceContext.Duplicate());
	if (OwnerAbility) SetSpecContext(SpecHandle, SourceContext);
	UMAAbilitySystemStatics::ApplyDamageExecutionConfig(SpecHandle, DamageConfig);
	return SpecHandle;
}

FMAResolvedDamage MASkillDamageResolver::Resolve(
	UAbilitySystemComponent& SourceASC,
	UMASkillAbility* OwnerAbility,
	const FGameplayEffectContextHandle& SourceContext,
	const FMASkillDamageConfig& DamageConfig,
	const FMASkillPayloadAccess& Payloads)
{
	FMAResolvedDamage ResolvedDamage;
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
		ResolvedDamage.DamageSpec = MakeDamageEffectSpec(
			SourceASC,
			OwnerAbility,
			SourceContext,
			bApplyDamageOverTime,
			AppliedExecutionConfig);

		if (ResolvedDamage.DamageSpec.IsValid() && ResolvedDamage.DamageSpec.Data.IsValid())
		{
			const FGameplayTag FinalDamageMultiplierTag = UMAAbilitySystemStatics::GetFinalDamageMultiplierTag();
			const float FinalDamageMultiplier = Payloads.Reader.GetScalarProduct(FinalDamageMultiplierTag);
			if (!FMath::IsNearlyEqual(FinalDamageMultiplier, 1.f))
			{
				ResolvedDamage.DamageSpec.Data->SetSetByCallerMagnitude(
					FinalDamageMultiplierTag,
					FinalDamageMultiplier);
			}

			const FGameplayTag DamageVarianceTag = UMAAbilitySystemStatics::GetDamageVarianceTag();
			const float DamageVariance = Payloads.Reader.GetScalarSum(DamageVarianceTag);
			if (!FMath::IsNearlyZero(DamageVariance))
			{
				ResolvedDamage.DamageSpec.Data->SetSetByCallerMagnitude(
					DamageVarianceTag,
					DamageVariance);
			}

			auto ForwardScalar = [&](const FGameplayTag& Tag, const float DefaultValue)
			{
				float Value = DefaultValue;
				if (Payloads.Reader.TryGetScalar(Tag, Value) && !FMath::IsNearlyEqual(Value, DefaultValue))
				{
					ResolvedDamage.DamageSpec.Data->SetSetByCallerMagnitude(Tag, Value);
				}
			};

			ForwardScalar(UMAAbilitySystemStatics::GetSkillFocusOffsetTag(), 0.f);
			ForwardScalar(UMAAbilitySystemStatics::GetSkillCriticalDamageOffsetTag(), 0.f);
		}

		if (bApplyDamageOverTime)
		{
			ApplyDamageOverTimeConfig(ResolvedDamage.DamageSpec, DamageConfig.DamageOverTime);
		}
	}

	for (const TObjectPtr<UMASkillStatusEffect>& StatusEffect : DamageConfig.StatusEffects)
	{
		if (!StatusEffect) continue;
		const int32 FirstNewEffectIndex = ResolvedDamage.StatusEffects.Num();
		StatusEffect->BuildResolvedEffect(SourceASC, OwnerAbility, ResolvedDamage.StatusEffects);
		for (int32 EffectIndex = FirstNewEffectIndex; EffectIndex < ResolvedDamage.StatusEffects.Num(); ++EffectIndex)
		{
			SetSpecContext(ResolvedDamage.StatusEffects[EffectIndex].SpecHandle, SourceContext);
		}
	}

	return ResolvedDamage;
}
