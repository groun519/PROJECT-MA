#include "GAS/Skill/MAGameplayEffect_SkillDamage.h"

#include "GAS/ExecCalc_DamageByAttribute.h"

UMAGameplayEffect_SkillDamage::UMAGameplayEffect_SkillDamage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayEffectExecutionDefinition& ExecutionDefinition = Executions.AddDefaulted_GetRef();
	ExecutionDefinition.CalculationClass = UExecCalc_DamageByAttribute::StaticClass();
}
