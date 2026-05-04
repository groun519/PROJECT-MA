#include "GAS/Skill/Damage/MAGameplayEffect_SkillDamageOverTime.h"

#include "GAS/Skill/Damage/ExecCalc_DamageByAttribute.h"

UMAGameplayEffect_SkillDamageOverTime::UMAGameplayEffect_SkillDamageOverTime()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	bExecutePeriodicEffectOnApplication = false;

	FGameplayEffectExecutionDefinition& ExecutionDefinition = Executions.AddDefaulted_GetRef();
	ExecutionDefinition.CalculationClass = UExecCalc_DamageByAttribute::StaticClass();
}
