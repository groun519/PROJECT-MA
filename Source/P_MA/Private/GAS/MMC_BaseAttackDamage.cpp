// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MMC_BaseAttackDamage.h"

#include "MAAbilitySystemStatics.h"
#include "MAGameplayAbilityTypes.h"
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

	CriticalChanceCaptureDef.AttributeToCapture = UMAAttributeSet::GetCriticalChanceAttribute();
	CriticalChanceCaptureDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	CriticalDamageCaptureDef.AttributeToCapture = UMAAttributeSet::GetCriticalDamageAttribute();
	CriticalDamageCaptureDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	RelevantAttributesToCapture.Add(DamageCaptureDef);
	RelevantAttributesToCapture.Add(ArmorCaptureDef);
	RelevantAttributesToCapture.Add(ArmorPenetrationCaptureDef);
	RelevantAttributesToCapture.Add(DamageVarianceCaptureDef);
	RelevantAttributesToCapture.Add(CriticalChanceCaptureDef);
	RelevantAttributesToCapture.Add(CriticalDamageCaptureDef);

	BehaviorModifierTag = UMAAbilitySystemStatics::GetBehaviorMultiplierTag();
	UtilityModifierTag = UMAAbilitySystemStatics::GetUtilityMultiplierTag();
	ElementalModifierTag = UMAAbilitySystemStatics::GetElementalMultiplierTag();
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

	float CriticalChance = 0.f;
	GetCapturedAttributeMagnitude(CriticalChanceCaptureDef, Spec, EvalParams, CriticalChance);
	float CriticalDamage = 0.f;
	GetCapturedAttributeMagnitude(CriticalDamageCaptureDef, Spec, EvalParams, CriticalDamage);

	float BehaviorBonus = Spec.GetSetByCallerMagnitude(BehaviorModifierTag,false,1.f);
	float UtilityBonus = Spec.GetSetByCallerMagnitude(UtilityModifierTag, false, 1.f);
	float ElementBonus = Spec.GetSetByCallerMagnitude(ElementalModifierTag, false, 1.f);
	
	// 방어력이 0 밑으로 내려가지 않도록 안전장치
	const float EffectiveArmor = FMath::Max(0.f, Armor - ArmorPenetration);

	const float MinMultiplier = 1.f - DamageVariance;
	const float MaxMultiplier = 1.f + DamageVariance;
	float RandomizedDamage = FMath::RandRange(MinMultiplier, MaxMultiplier) * AttackDamage;

	bool bIsCriticalHit = FMath::RandRange(0.f, 1.f) <= CriticalChance;
	if (bIsCriticalHit)
	{
		RandomizedDamage *= CriticalDamage;
	}
	FGameplayEffectContextHandle ContextHandle = Spec.GetContext();
	if (FMAGameplayEffectContext* MAContext = static_cast<FMAGameplayEffectContext*>(ContextHandle.Get()))
	{
		MAContext->SetIsCriticalHit(bIsCriticalHit);
	}
	
	const float Damage = RandomizedDamage * (1.f - (EffectiveArmor / (EffectiveArmor + 100.f)));
	const float FinalDamage = Damage * UtilityBonus * ElementBonus * BehaviorBonus;
	//const float FinalDamage = Damage * ElementBonus * BehaviorBonus * (UtilityBonus+1.f);
	//UE_LOG(LogTemp, Warning, TEXT("Damage: %f"), FMath::RoundToFloat(FinalDamage));
	return FMath::RoundToFloat(-FinalDamage);
}


