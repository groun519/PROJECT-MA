#include "GAS/MAGameplayEffect_MonsterWaveStatScale.h"

#include "AbilitySystemComponent.h"
#include "GAS/MAAttributeSet.h"

UMAGameplayEffect_MonsterWaveStatScale::UMAGameplayEffect_MonsterWaveStatScale()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat StatCoefficientMagnitude;
	StatCoefficientMagnitude.DataName = GetStatCoefficientDataName();

	FGameplayModifierInfo MaxHealthModifier;
	MaxHealthModifier.Attribute = UMAAttributeSet::GetMaxHealthAttribute();
	MaxHealthModifier.ModifierOp = EGameplayModOp::Multiplicitive;
	MaxHealthModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(StatCoefficientMagnitude);
	Modifiers.Add(MaxHealthModifier);

	FGameplayModifierInfo AttackModifier;
	AttackModifier.Attribute = UMAAttributeSet::GetAttackAttribute();
	AttackModifier.ModifierOp = EGameplayModOp::Multiplicitive;
	AttackModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(StatCoefficientMagnitude);
	Modifiers.Add(AttackModifier);
}

FName UMAGameplayEffect_MonsterWaveStatScale::GetStatCoefficientDataName()
{
	static const FName DataName(TEXT("Data.Monster.WaveStatCoefficient"));
	return DataName;
}

void UMAGameplayEffect_MonsterWaveStatScale::ApplyTo(UAbilitySystemComponent& AbilitySystemComponent, float StatCoefficient)
{
	if (FMath::IsNearlyEqual(StatCoefficient, 1.f)) return;

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent.MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent.MakeOutgoingSpec(StaticClass(), 1.f, EffectContext);
	if (!SpecHandle.IsValid()) return;

	SpecHandle.Data->SetSetByCallerMagnitude(GetStatCoefficientDataName(), StatCoefficient);
	AbilitySystemComponent.ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
