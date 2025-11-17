// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MMC_BaseAttackDamage.h"
#include "GAS/MAAttributeSet.h"

UMMC_BaseAttackDamage::UMMC_BaseAttackDamage()
{
	DamageCaptureDef.AttributeToCapture = UMAAttributeSet::GetAttackAttribute();
	DamageCaptureDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	ArmorCaptureDef.AttributeToCapture = UMAAttributeSet::GetArmorAttribute();
	ArmorCaptureDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;

	ArmorPenetrationCaptureDef.AttributeToCapture = UMAAttributeSet::GetArmorPenetrationAttribute();
	ArmorPenetrationCaptureDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	DamageVarianceCaptureDef.AttributeToCapture = UMAAttributeSet::GetDamageVarianceAttribute();
	DamageVarianceCaptureDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	RelevantAttributesToCapture.Add(DamageCaptureDef);
	RelevantAttributesToCapture.Add(ArmorCaptureDef);
	RelevantAttributesToCapture.Add(ArmorPenetrationCaptureDef);
	RelevantAttributesToCapture.Add(DamageVarianceCaptureDef);

	DamageModifierTag = FGameplayTag::RequestGameplayTag("Data.Damage.UtilityModifier");
	ElementalMultiplierTag = FGameplayTag::RequestGameplayTag("Data.Damage.ElementalModifier");
}

float UMMC_BaseAttackDamage::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float AttackDamage = 0.f;
	GetCapturedAttributeMagnitude(DamageCaptureDef, Spec, EvalParams, AttackDamage);

	float Armor = 0.f;
	GetCapturedAttributeMagnitude(ArmorCaptureDef, Spec, EvalParams, Armor);

	float ArmorPenetration = 0.f;
	GetCapturedAttributeMagnitude(ArmorPenetrationCaptureDef, Spec, EvalParams, ArmorPenetration);

	float DamageVariance = 0.f;
	GetCapturedAttributeMagnitude(DamageVarianceCaptureDef,Spec, EvalParams, DamageVariance);

	float UtilityBonus = Spec.GetSetByCallerMagnitude(DamageModifierTag, false, 0.f);
	float ElementBonus = Spec.GetSetByCallerMagnitude(ElementalMultiplierTag, false, 1.f);
	
	// 방어력이 0 밑으로 내려가지 않도록 안전장치
	const float EffectiveArmor = FMath::Max(0.f, Armor - ArmorPenetration);

	const float MinMultiplier = 1.f - DamageVariance;
	const float MaxMultiplier = 1.f + DamageVariance;
	const float RandomizedDamage = FMath::RandRange(MinMultiplier, MaxMultiplier) * AttackDamage;
	
	const float Damage = RandomizedDamage * (1.f - (EffectiveArmor / (EffectiveArmor + 100.f)));
	const float FinalDamage = Damage * (1.0f + UtilityBonus) * ElementBonus;

	return -FinalDamage;
}


