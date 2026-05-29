#include "GAS/Elemental/MAGameplayEffect_TemperatureSlow.h"

#include "GAS/MAAttributeSet.h"

UMAGameplayEffect_TemperatureSlow::UMAGameplayEffect_TemperatureSlow()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	FSetByCallerFloat SlowMultiplierMagnitude;
	SlowMultiplierMagnitude.DataName = GetSlowMultiplierDataName();

	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute = UMAAttributeSet::GetSlowMultiplierAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Multiplicitive;
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SlowMultiplierMagnitude);
	Modifiers.Add(ModifierInfo);
}

FName UMAGameplayEffect_TemperatureSlow::GetSlowMultiplierDataName()
{
	static const FName DataName(TEXT("Data.Elemental.TemperatureSlowMultiplier"));
	return DataName;
}
