#include "GAS/Skill/MASkillDamageConfig.h"

#include "GAS/Skill/CrowdControl/MASkillCrowdControl.h"
#include "GAS/Skill/MAGameplayEffect_SkillDamage.h"
#include "GAS/Skill/MASkillAbility.h"

namespace MASkillResolvedHitEffects
{
	FResolvedSkillHitEffects BuildResolvedHitEffects(UMASkillAbility& OwnerAbility, const FMASkillDamageConfig& DamageConfig)
	{
		FResolvedSkillHitEffects ResolvedHitEffects;
		ResolvedHitEffects.TargetRelationMask = DamageConfig.TargetRelationMask;

		const FMADamageExecutionConfig ExecutionConfig = DamageConfig.ToExecutionConfig();
		ResolvedHitEffects.DamageSpec = OwnerAbility.MakeDamageEffectSpec(
			UMAGameplayEffect_SkillDamage::StaticClass(),
			1,
			ExecutionConfig.HasValues() ? &ExecutionConfig : nullptr);

		for (const TObjectPtr<UMASkillCrowdControl>& CrowdControl : DamageConfig.CrowdControls)
		{
			if (!CrowdControl) continue;
			CrowdControl->BuildResolvedEffect(OwnerAbility, ResolvedHitEffects.CrowdControlEffects);
		}

		return ResolvedHitEffects;
	}
}
