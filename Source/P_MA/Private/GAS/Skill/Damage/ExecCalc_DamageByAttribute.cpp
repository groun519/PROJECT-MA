#include "GAS/Skill/Damage/ExecCalc_DamageByAttribute.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "UObject/UnrealType.h"

UExecCalc_DamageByAttribute::UExecCalc_DamageByAttribute()
{
	for (TFieldIterator<FProperty> PropertyIt(UMAAttributeSet::StaticClass(), EFieldIteratorFlags::IncludeSuper);
		PropertyIt;
		++PropertyIt)
	{
		FProperty* Property = *PropertyIt;
		if (!FGameplayAttribute::IsGameplayAttributeDataProperty(Property)) continue;

		const FGameplayAttribute Attribute(Property);
		const FGameplayEffectAttributeCaptureDefinition SourceDefinition(
			Attribute,
			EGameplayEffectAttributeCaptureSource::Source,
			false);
		const FGameplayEffectAttributeCaptureDefinition TargetDefinition(
			Attribute,
			EGameplayEffectAttributeCaptureSource::Target,
			false);

		FAttributeCaptureDefinitions& CaptureDefinitions = AttributeCaptureDefinitions.Add(Attribute);
		CaptureDefinitions.Source = SourceDefinition;
		CaptureDefinitions.Target = TargetDefinition;
		CaptureDefinitions.SourceCoefficientName = UMAAbilitySystemStatics::GetDamageAttributeCoefficientName(
			EMACoefficientSource::Source,
			Attribute);
		CaptureDefinitions.TargetCoefficientName = UMAAbilitySystemStatics::GetDamageAttributeCoefficientName(
			EMACoefficientSource::Target,
			Attribute);
		RelevantAttributesToCapture.Add(SourceDefinition);
		RelevantAttributesToCapture.Add(TargetDefinition);
	}

	BehaviorModifierTag = UMAAbilitySystemStatics::GetBehaviorMultiplierTag();
	DamageVarianceTag = UMAAbilitySystemStatics::GetDamageVarianceTag();
}

const FGameplayEffectAttributeCaptureDefinition* UExecCalc_DamageByAttribute::FindCaptureDefinition(
	EMACoefficientSource Side,
	const FGameplayAttribute& Attribute) const
{
	const FAttributeCaptureDefinitions* CaptureDefinitions = AttributeCaptureDefinitions.Find(Attribute);
	if (!CaptureDefinitions) return nullptr;

	return Side == EMACoefficientSource::Source
		? &CaptureDefinitions->Source
		: &CaptureDefinitions->Target;
}

void UExecCalc_DamageByAttribute::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	if (FMAGameplayEffectContext* MutableMAContext =
		static_cast<FMAGameplayEffectContext*>(Spec.GetContext().Get()))
	{
		MutableMAContext->SetDisplayMagnitude(0.f);
	}

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	auto CaptureMagnitude = [&](const FGameplayEffectAttributeCaptureDefinition& CaptureDef)
	{
		float Value = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvalParams, Value);
		return Value;
	};

	auto CaptureAttributeMagnitude = [&](EMACoefficientSource Side, const FGameplayAttribute& Attribute)
	{
		const FGameplayEffectAttributeCaptureDefinition* CaptureDefinition = FindCaptureDefinition(Side, Attribute);
		return CaptureDefinition ? CaptureMagnitude(*CaptureDefinition) : 0.f;
	};

	float BaseDamage = Spec.GetSetByCallerMagnitude(UMAAbilitySystemStatics::GetDamageBaseTag(), false, 0.f);
	bool bHasConfiguredBaseDamage = !FMath::IsNearlyZero(BaseDamage);

	auto AddAttributeCoefficient = [&](
		FName CoefficientName,
		const FGameplayEffectAttributeCaptureDefinition& CaptureDefinition)
	{
		const float Coefficient = Spec.GetSetByCallerMagnitude(CoefficientName, false, 0.f);
		if (FMath::IsNearlyZero(Coefficient)) return;

		bHasConfiguredBaseDamage = true;
		BaseDamage += CaptureMagnitude(CaptureDefinition) * Coefficient;
	};

	for (const TPair<FGameplayAttribute, FAttributeCaptureDefinitions>& Pair : AttributeCaptureDefinitions)
	{
		AddAttributeCoefficient(Pair.Value.SourceCoefficientName, Pair.Value.Source);
		AddAttributeCoefficient(Pair.Value.TargetCoefficientName, Pair.Value.Target);
	}

	if (!bHasConfiguredBaseDamage)
	{
		return;
	}

	const FMAGameplayEffectContext* MAContext = static_cast<const FMAGameplayEffectContext*>(Spec.GetContext().Get());
	if (!MAContext || !MAContext->GetDamageTypeTag().IsValid()) return;

	const FGameplayTag DamageTypeTag = MAContext->GetDamageTypeTag();
	const bool bCanCriticalHit = DamageTypeTag == UMAAbilitySystemStatics::GetDefaultDamageTypeTag();
	const bool bIsFixedDamage = DamageTypeTag == UMAAbilitySystemStatics::GetFixedDamageTypeTag();
	auto EmitZeroDamage = [&]()
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UMAAttributeSet::GetHealthAttribute(),
			EGameplayModOp::Additive,
			0.f));
	};

	BaseDamage = FMath::Max(0.f, BaseDamage);
	if (BaseDamage <= 0.f)
	{
		if (bCanCriticalHit) EmitZeroDamage();
		return;
	}
	if (DamageTypeTag.MatchesTag(UMAAbilitySystemStatics::GetHealDamageTypeTag()))
	{
		const float FinalHeal = FMath::RoundToFloat(BaseDamage);
		if (FinalHeal <= 0.f) return;

		if (FMAGameplayEffectContext* MutableMAContext = static_cast<FMAGameplayEffectContext*>(Spec.GetContext().Get()))
		{
			MutableMAContext->SetDisplayMagnitude(FinalHeal);
		}
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UMAAttributeSet::GetHealthAttribute(),
			EGameplayModOp::Additive,
			FinalHeal));
		return;
	}

	if (DamageTypeTag.MatchesTag(UMAAbilitySystemStatics::GetFireDamageTypeTag()))
	{
		const float FinalFireDamage = FMath::RoundToFloat(BaseDamage);
		if (FinalFireDamage <= 0.f) return;

		if (FMAGameplayEffectContext* MutableMAContext = static_cast<FMAGameplayEffectContext*>(Spec.GetContext().Get()))
		{
			MutableMAContext->SetDisplayMagnitude(FinalFireDamage);
		}
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UMAAttributeSet::GetTemperatureAttribute(),
			EGameplayModOp::Additive,
			FinalFireDamage));
		return;
	}

	if (DamageTypeTag.MatchesTag(UMAAbilitySystemStatics::GetIceDamageTypeTag()))
	{
		const float FinalIceDamage = FMath::RoundToFloat(BaseDamage);
		if (FinalIceDamage <= 0.f) return;

		if (FMAGameplayEffectContext* MutableMAContext = static_cast<FMAGameplayEffectContext*>(Spec.GetContext().Get()))
		{
			MutableMAContext->SetDisplayMagnitude(FinalIceDamage);
		}
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UMAAttributeSet::GetTemperatureAttribute(),
			EGameplayModOp::Additive,
			-FinalIceDamage));
		return;
	}

	const float BehaviorBonus = Spec.GetSetByCallerMagnitude(BehaviorModifierTag, false, 1.f);
	const float FinalDamageMultiplier = Spec.GetSetByCallerMagnitude(UMAAbilitySystemStatics::GetFinalDamageMultiplierTag(), false, 1.f);

	auto ApplyHealthDamage = [&](float FinalDamage)
	{
		if (FMAGameplayEffectContext* MutableMAContext = static_cast<FMAGameplayEffectContext*>(Spec.GetContext().Get()))
		{
			MutableMAContext->SetDisplayMagnitude(FinalDamage);
		}

		const float CurrentShield = FMath::Max(0.f, CaptureAttributeMagnitude(
			EMACoefficientSource::Target,
			UMAAttributeSet::GetShieldAttribute()));
		const float ShieldDamage = FMath::Min(CurrentShield, FinalDamage);
		const float HealthDamage = FinalDamage - ShieldDamage;
		if (ShieldDamage > 0.f)
		{
			OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
				UMAAttributeSet::GetShieldAttribute(),
				EGameplayModOp::Additive,
				-ShieldDamage));
		}
		if (HealthDamage > 0.f)
		{
			OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
				UMAAttributeSet::GetHealthAttribute(),
				EGameplayModOp::Additive,
				-HealthDamage));
		}
		else
		{
			OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
				UMAAttributeSet::GetHealthAttribute(),
				EGameplayModOp::Additive,
				0.f));
		}
	};

	if (bIsFixedDamage)
	{
		const float FinalDamage = FMath::RoundToFloat(BaseDamage * BehaviorBonus * FinalDamageMultiplier);
		if (FinalDamage <= 0.f) return;

		if (FMAGameplayEffectContext* MutableMAContext = static_cast<FMAGameplayEffectContext*>(Spec.GetContext().Get()))
		{
			MutableMAContext->SetCriticalResult(EMADamageCriticalResult::None);
		}
		ApplyHealthDamage(FinalDamage);
		return;
	}

	const float Armor = CaptureAttributeMagnitude(
		EMACoefficientSource::Target,
		UMAAttributeSet::GetArmorAttribute());
	const float ArmorPenetration = CaptureAttributeMagnitude(
		EMACoefficientSource::Source,
		UMAAttributeSet::GetArmorPenetrationAttribute());
	const float DamageVariance = FMath::Max(0.f, Spec.GetSetByCallerMagnitude(DamageVarianceTag, false, 0.f));
	const float Focus = FMath::Clamp(
		CaptureAttributeMagnitude(EMACoefficientSource::Source, UMAAttributeSet::GetFocusAttribute())
		+ Spec.GetSetByCallerMagnitude(UMAAbilitySystemStatics::GetSkillFocusOffsetTag(), false, 0.f),
		-1.f,
		1.f);
	const float CriticalDamage = CaptureAttributeMagnitude(
		EMACoefficientSource::Source,
		UMAAttributeSet::GetCriticalDamageAttribute())
		+ Spec.GetSetByCallerMagnitude(UMAAbilitySystemStatics::GetSkillCriticalDamageOffsetTag(), false, 0.f);
	const float ReverseCriticalDamage = CaptureAttributeMagnitude(
		EMACoefficientSource::Source,
		UMAAttributeSet::GetReverseCriticalDamageAttribute());

	const float EffectiveArmor = FMath::Max(0.f, Armor - ArmorPenetration);
	const float MinMultiplier = 1.f - DamageVariance;
	const float MaxMultiplier = 1.f + DamageVariance;
	float RandomizedDamage = FMath::RandRange(MinMultiplier, MaxMultiplier) * BaseDamage;

	EMADamageCriticalResult CriticalResult = EMADamageCriticalResult::None;
	if (bCanCriticalHit && Focus > 0.f && FMath::FRand() <= Focus)
	{
		CriticalResult = EMADamageCriticalResult::Critical;
		RandomizedDamage *= CriticalDamage;
	}
	else if (bCanCriticalHit && Focus < 0.f && FMath::FRand() <= -Focus)
	{
		CriticalResult = EMADamageCriticalResult::ReverseCritical;
		RandomizedDamage *= ReverseCriticalDamage;
	}

	if (FMAGameplayEffectContext* MutableMAContext = static_cast<FMAGameplayEffectContext*>(Spec.GetContext().Get()))
	{
		MutableMAContext->SetCriticalResult(CriticalResult);
	}

	const float DamageAfterArmor = RandomizedDamage * (1.f - (EffectiveArmor / (EffectiveArmor + 100.f)));
	const float FinalDamage = FMath::RoundToFloat(DamageAfterArmor * BehaviorBonus * FinalDamageMultiplier);
	if (FinalDamage <= 0.f)
	{
		EmitZeroDamage();
		return;
	}

	ApplyHealthDamage(FinalDamage);
}
