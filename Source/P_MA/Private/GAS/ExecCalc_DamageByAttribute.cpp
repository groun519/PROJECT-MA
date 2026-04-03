#include "GAS/ExecCalc_DamageByAttribute.h"

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
	InitCaptureDef(SourceCriticalChanceDef, UMAAttributeSet::GetCriticalChanceAttribute(), EGameplayEffectAttributeCaptureSource::Source);
	InitCaptureDef(SourceCriticalDamageDef, UMAAttributeSet::GetCriticalDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source);

	InitCaptureDef(TargetHealthDef, UMAAttributeSet::GetHealthAttribute(), EGameplayEffectAttributeCaptureSource::Target);
	InitCaptureDef(TargetMaxHealthDef, UMAAttributeSet::GetMaxHealthAttribute(), EGameplayEffectAttributeCaptureSource::Target);
	InitCaptureDef(TargetAttackDef, UMAAttributeSet::GetAttackAttribute(), EGameplayEffectAttributeCaptureSource::Target);
	InitCaptureDef(TargetMoveSpeedDef, UMAAttributeSet::GetMoveSpeedAttribute(), EGameplayEffectAttributeCaptureSource::Target);
	InitCaptureDef(TargetAttackSpeedDef, UMAAttributeSet::GetAttackSpeedAttribute(), EGameplayEffectAttributeCaptureSource::Target);
	InitCaptureDef(TargetArmorDef, UMAAttributeSet::GetArmorAttribute(), EGameplayEffectAttributeCaptureSource::Target);
	InitCaptureDef(TargetArmorPenetrationDef, UMAAttributeSet::GetArmorPenetrationAttribute(), EGameplayEffectAttributeCaptureSource::Target);
	InitCaptureDef(TargetCriticalChanceDef, UMAAttributeSet::GetCriticalChanceAttribute(), EGameplayEffectAttributeCaptureSource::Target);
	InitCaptureDef(TargetCriticalDamageDef, UMAAttributeSet::GetCriticalDamageAttribute(), EGameplayEffectAttributeCaptureSource::Target);

	InitCaptureDef(SourceDamageVarianceDef, UMAAttributeSet::GetDamageVarianceAttribute(), EGameplayEffectAttributeCaptureSource::Source);

	BehaviorModifierTag = UMAAbilitySystemStatics::GetBehaviorMultiplierTag();
	UtilityModifierTag = UMAAbilitySystemStatics::GetUtilityMultiplierTag();
	ElementalModifierTag = UMAAbilitySystemStatics::GetElementalMultiplierTag();
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
		case EMADamageAttribute::CriticalChance: return bSource ? SourceCriticalChanceDef : TargetCriticalChanceDef;
		case EMADamageAttribute::CriticalDamage: return bSource ? SourceCriticalDamageDef : TargetCriticalDamageDef;
		}

		return bSource ? SourceAttackDef : TargetAttackDef;
	};

	float BaseDamage = Spec.GetSetByCallerMagnitude(UMAAbilitySystemStatics::GetDamageBaseTag(), false, 0.f);
	bool bHasConfiguredBaseDamage = !FMath::IsNearlyZero(BaseDamage);

	for (int32 SideIndex = 0; SideIndex < 2; ++SideIndex)
	{
		const EMADamageAttributeSide Side = SideIndex == 0 ? EMADamageAttributeSide::Source : EMADamageAttributeSide::Target;
		for (int32 AttributeIndex = 0; AttributeIndex <= static_cast<int32>(EMADamageAttribute::CriticalDamage); ++AttributeIndex)
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

	const float Armor = CaptureMagnitude(TargetArmorDef);
	const float ArmorPenetration = CaptureMagnitude(SourceArmorPenetrationDef);
	const float DamageVariance = CaptureMagnitude(SourceDamageVarianceDef);
	const float CriticalChance = CaptureMagnitude(SourceCriticalChanceDef);
	const float CriticalDamage = CaptureMagnitude(SourceCriticalDamageDef);

	const float BehaviorBonus = Spec.GetSetByCallerMagnitude(BehaviorModifierTag, false, 1.f);
	const float UtilityBonus = Spec.GetSetByCallerMagnitude(UtilityModifierTag, false, 1.f);
	const float ElementBonus = Spec.GetSetByCallerMagnitude(ElementalModifierTag, false, 1.f);

	const float EffectiveArmor = FMath::Max(0.f, Armor - ArmorPenetration);
	const float MinMultiplier = 1.f - DamageVariance;
	const float MaxMultiplier = 1.f + DamageVariance;
	float RandomizedDamage = FMath::RandRange(MinMultiplier, MaxMultiplier) * BaseDamage;

	bool bIsCriticalHit = FMath::FRand() <= CriticalChance;
	if (bIsCriticalHit)
	{
		RandomizedDamage *= CriticalDamage;
	}

	if (FMAGameplayEffectContext* MAContext = static_cast<FMAGameplayEffectContext*>(Spec.GetContext().Get()))
	{
		MAContext->SetIsCriticalHit(bIsCriticalHit);
	}

	const float DamageAfterArmor = RandomizedDamage * (1.f - (EffectiveArmor / (EffectiveArmor + 100.f)));
	const float FinalDamage = FMath::RoundToFloat(DamageAfterArmor * UtilityBonus * ElementBonus * BehaviorBonus);
	if (FinalDamage <= 0.f) return;

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UMAAttributeSet::GetHealthAttribute(),
		EGameplayModOp::Additive,
		-FinalDamage));
}
