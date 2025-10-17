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

	RelevantAttributesToCapture.Add(DamageCaptureDef);
	RelevantAttributesToCapture.Add(ArmorCaptureDef);
	RelevantAttributesToCapture.Add(ArmorPenetrationCaptureDef);
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

	// 방어력이 0 밑으로 내려가지 않도록 안전장치
	const float EffectiveArmor = FMath::Max(0.f, Armor - ArmorPenetration);
	
	const float Damage = AttackDamage * (1.f - (EffectiveArmor / (EffectiveArmor + 100.f)));

	return -Damage;
}


