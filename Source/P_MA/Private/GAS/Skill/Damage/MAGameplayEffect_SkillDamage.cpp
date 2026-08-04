#include "GAS/Skill/Damage/MAGameplayEffect_SkillDamage.h"

#include "GAS/Skill/Damage/ExecCalc_DamageByAttribute.h"

UMAGameplayEffect_SkillDamage::UMAGameplayEffect_SkillDamage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayEffectExecutionDefinition& ExecutionDefinition = Executions.AddDefaulted_GetRef();
	ExecutionDefinition.CalculationClass = UExecCalc_DamageByAttribute::StaticClass();
}
