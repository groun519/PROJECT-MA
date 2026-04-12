#include "GAS/Skill/CrowdControl/MAGameplayEffect_StatusEffectAttribute.h"

#include "GAS/MAAttributeSet.h"

UMAGameplayEffect_StatusEffectAttribute::UMAGameplayEffect_StatusEffectAttribute()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	RebuildModifiers();
}

void UMAGameplayEffect_StatusEffectAttribute::SetTargetAttribute(FGameplayAttribute InAttribute)
{
	TargetAttribute = InAttribute;
	RebuildModifiers();
}

void UMAGameplayEffect_StatusEffectAttribute::SetModifierOp(EGameplayModOp::Type InModifierOp)
{
	ModifierOp = InModifierOp;
	RebuildModifiers();
}

void UMAGameplayEffect_StatusEffectAttribute::SetMagnitude(float InMagnitude)
{
	Magnitude = InMagnitude;
	RebuildModifiers();
}

void UMAGameplayEffect_StatusEffectAttribute::PostLoad()
{
	Super::PostLoad();
	RebuildModifiers();
}

#if WITH_EDITOR
void UMAGameplayEffect_StatusEffectAttribute::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RebuildModifiers();
}
#endif

void UMAGameplayEffect_StatusEffectAttribute::RebuildModifiers()
{
	Modifiers.Reset();
	if (!TargetAttribute.IsValid()) return;

	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute = TargetAttribute;
	ModifierInfo.ModifierOp = ModifierOp;
	ModifierInfo.ModifierMagnitude = FScalableFloat(Magnitude);
	Modifiers.Add(ModifierInfo);
}

UMAGameplayEffect_StatusEffectSlow::UMAGameplayEffect_StatusEffectSlow()
{
	TargetAttribute = UMAAttributeSet::GetMoveSpeedAttribute();
	ModifierOp = EGameplayModOp::Multiplicitive;
	Magnitude = 0.8f;
	RebuildModifiers();
}

UMAGameplayEffect_StatusEffectHaste::UMAGameplayEffect_StatusEffectHaste()
{
	TargetAttribute = UMAAttributeSet::GetMoveSpeedAttribute();
	ModifierOp = EGameplayModOp::Multiplicitive;
	Magnitude = 1.2f;
	RebuildModifiers();
}
