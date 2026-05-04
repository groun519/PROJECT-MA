#include "GAS/Skill/MAGameplayEffect_SkillDamageOverTime.h"

#include "GAS/ExecCalc_DamageByAttribute.h"

UMAGameplayEffect_SkillDamageOverTime::UMAGameplayEffect_SkillDamageOverTime()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	bExecutePeriodicEffectOnApplication = false;

	FGameplayEffectExecutionDefinition& ExecutionDefinition = Executions.AddDefaulted_GetRef();
	ExecutionDefinition.CalculationClass = UExecCalc_DamageByAttribute::StaticClass();
}
