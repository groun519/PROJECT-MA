#include "GAS/Skill/Damage/ExecCalc_DamageByAttribute.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/MAGameplayAbilityTypes.h"

UExecCalc_DamageByAttribute::UExecCalc_DamageByAttribute()
{
	auto InitCaptureDef = [this](
		FGameplayEffectAttributeCaptureDefinition& CaptureDef,
		const FGameplayAttribute& Attribute,
		EGameplayEffectAttributeCaptureSource CaptureSource)
	{
		CaptureDef.AttributeToCapture = Attribute;
		CaptureDef.AttributeSource = CaptureSource;
		CaptureDef.bSnapshot = false;
		RelevantAttributesToCapture.Add(CaptureDef);
	};

	InitCaptureDef(SourceHealthDef, UMAAttributeSet::GetHealthAttribute(), EGameplayEffectAttributeCaptureSource::Source);
	InitCaptureDef(SourceMaxHealthDef, UMAAttributeSet::GetMaxHealthAttribute(), EGameplayEffectAttributeCaptureSource::Source);
	InitCaptureDef(SourceAttackDef, UMAAttributeSet::GetAttackAttribute(), EGameplayEffectAttributeCaptureSource::Source);
	InitCaptureDef(SourceMoveSpeedDef, UMAAttributeSet::GetMoveSpeedAttribute(), EGameplayEffectAttributeCaptureSource::Source);
	InitCaptureDef(SourceAttackSpeedDef, UMAAttributeSet::GetAttackSpeedAttribute(), EGameplayEffectAttributeCaptureSource::Source);
	InitCaptureDef(SourceArmorDef, UMAAttributeSet::GetArmorAttribute(), EGameplayEffectAttributeCaptureSource::Source);
	InitCaptureDef(SourceArmorPenetrationDef, UMAAttributeSet::GetArmorPenetrationAttribute(), EGameplayEffectAttributeCaptureSource::Source);
	InitCaptureDef(SourceFocusDef, UMAAttributeSet::GetFocusAttribute(), EGameplayEffectAttributeCaptureSource::Source);
	InitCaptureDef(SourceCriticalDamageDef, UMAAttributeSet::GetCriticalDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source);
	InitCaptureDef(SourceReverseCriticalDamageDef, UMAAttributeSet::GetReverseCriticalDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source);

	InitCaptureDef(TargetHealthDef, UMAAttributeSet::GetHealthAttribute(), EGameplayEffectAttributeCaptureSource::Target);
	InitCaptureDef(TargetMaxHealthDef, UMAAttributeSet::GetMaxHealthAttribute(), EGameplayEffectAttributeCaptureSource::Target);
	InitCaptureDef(TargetShieldDef, UMAAttributeSet::GetShieldAttribute(), EGameplayEffectAttributeCaptureSource::Target);
	InitCaptureDef(TargetAttackDef, UMAAttributeSet::GetAttackAttribute(), EGameplayEffectAttributeCaptureSource::Target);
	InitCaptureDef(TargetMoveSpeedDef, UMAAttributeSet::GetMoveSpeedAttribute(), EGameplayEffectAttributeCaptureSource::Target);
	InitCaptureDef(TargetAttackSpeedDef, UMAAttributeSet::GetAttackSpeedAttribute(), EGameplayEffectAttributeCaptureSource::Target);
	InitCaptureDef(TargetArmorDef, UMAAttributeSet::GetArmorAttribute(), EGameplayEffectAttributeCaptureSource::Target);
	InitCaptureDef(TargetArmorPenetrationDef, UMAAttributeSet::GetArmorPenetrationAttribute(), EGameplayEffectAttributeCaptureSource::Target);
	InitCaptureDef(TargetFocusDef, UMAAttributeSet::GetFocusAttribute(), EGameplayEffectAttributeCaptureSource::Target);
	InitCaptureDef(TargetCriticalDamageDef, UMAAttributeSet::GetCriticalDamageAttribute(), EGameplayEffectAttributeCaptureSource::Target);
	InitCaptureDef(TargetReverseCriticalDamageDef, UMAAttributeSet::GetReverseCriticalDamageAttribute(), EGameplayEffectAttributeCaptureSource::Target);

	BehaviorModifierTag = UMAAbilitySystemStatics::GetBehaviorMultiplierTag();
	DamageVarianceTag = UMAAbilitySystemStatics::GetDamageVarianceTag();
}

void UExecCalc_DamageByAttribute::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	auto CaptureMagnitude = [&](const FGameplayEffectAttributeCaptureDefinition& CaptureDef)
	{
		float Value = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvalParams, Value);
		return Value;
	};

	auto GetCaptureDef = [&](EMADamageAttributeSide Side, EMADamageAttribute Attribute) -> const FGameplayEffectAttributeCaptureDefinition&
	{
		const bool bSource = Side == EMADamageAttributeSide::Source;
		switch (Attribute)
		{
		case EMADamageAttribute::Health: return bSource ? SourceHealthDef : TargetHealthDef;
		case EMADamageAttribute::MaxHealth: return bSource ? SourceMaxHealthDef : TargetMaxHealthDef;
		case EMADamageAttribute::Attack: return bSource ? SourceAttackDef : TargetAttackDef;
		case EMADamageAttribute::MoveSpeed: return bSource ? SourceMoveSpeedDef : TargetMoveSpeedDef;
		case EMADamageAttribute::AttackSpeed: return bSource ? SourceAttackSpeedDef : TargetAttackSpeedDef;
		case EMADamageAttribute::Armor: return bSource ? SourceArmorDef : TargetArmorDef;
		case EMADamageAttribute::ArmorPenetration: return bSource ? SourceArmorPenetrationDef : TargetArmorPenetrationDef;
		case EMADamageAttribute::Focus: return bSource ? SourceFocusDef : TargetFocusDef;
		case EMADamageAttribute::CriticalDamage: return bSource ? SourceCriticalDamageDef : TargetCriticalDamageDef;
		case EMADamageAttribute::ReverseCriticalDamage: return bSource ? SourceReverseCriticalDamageDef : TargetReverseCriticalDamageDef;
		}

		return bSource ? SourceAttackDef : TargetAttackDef;
	};

	float BaseDamage = Spec.GetSetByCallerMagnitude(UMAAbilitySystemStatics::GetDamageBaseTag(), false, 0.f);
	bool bHasConfiguredBaseDamage = !FMath::IsNearlyZero(BaseDamage);

	for (int32 SideIndex = 0; SideIndex < 2; ++SideIndex)
	{
		const EMADamageAttributeSide Side = SideIndex == 0 ? EMADamageAttributeSide::Source : EMADamageAttributeSide::Target;
		for (int32 AttributeIndex = 0; AttributeIndex <= static_cast<int32>(EMADamageAttribute::ReverseCriticalDamage); ++AttributeIndex)
		{
			const EMADamageAttribute Attribute = static_cast<EMADamageAttribute>(AttributeIndex);
			const float Coefficient = Spec.GetSetByCallerMagnitude(
				UMAAbilitySystemStatics::GetDamageAttributeCoefficientTag(Side, Attribute),
				false,
				0.f);

			if (FMath::IsNearlyZero(Coefficient)) continue;

			bHasConfiguredBaseDamage = true;
			BaseDamage += CaptureMagnitude(GetCaptureDef(Side, Attribute)) * Coefficient;
		}
	}

	if (!bHasConfiguredBaseDamage)
	{
		return;
	}

	BaseDamage = FMath::Max(0.f, BaseDamage);
	if (BaseDamage <= 0.f) return;

	const FMAGameplayEffectContext* MAContext = static_cast<const FMAGameplayEffectContext*>(Spec.GetContext().Get());
	const FGameplayTag DamageTypeTag = MAContext && MAContext->GetDamageTypeTag().IsValid()
		? MAContext->GetDamageTypeTag()
		: UMAAbilitySystemStatics::GetDefaultDamageTypeTag();
	const bool bCanCriticalHit = DamageTypeTag == UMAAbilitySystemStatics::GetDefaultDamageTypeTag();
	const bool bIsFixedDamage = DamageTypeTag == UMAAbilitySystemStatics::GetFixedDamageTypeTag();
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

		const float CurrentShield = FMath::Max(0.f, CaptureMagnitude(TargetShieldDef));
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

	const float Armor = CaptureMagnitude(TargetArmorDef);
	const float ArmorPenetration = CaptureMagnitude(SourceArmorPenetrationDef);
	const float DamageVariance = FMath::Max(0.f, Spec.GetSetByCallerMagnitude(DamageVarianceTag, false, 0.f));
	const float Focus = FMath::Clamp(CaptureMagnitude(SourceFocusDef), -1.f, 1.f);
	const float CriticalDamage = CaptureMagnitude(SourceCriticalDamageDef);
	const float ReverseCriticalDamage = CaptureMagnitude(SourceReverseCriticalDamageDef);

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
	if (FinalDamage <= 0.f) return;

	ApplyHealthDamage(FinalDamage);
}
