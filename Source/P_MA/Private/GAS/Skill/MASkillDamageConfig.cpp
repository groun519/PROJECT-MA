#include "GAS/Skill/MASkillDamageConfig.h"

#include "GAS/Skill/StatusEffect/MASkillStatusEffect.h"
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

		for (const TObjectPtr<UMASkillStatusEffect>& StatusEffect : DamageConfig.StatusEffects)
		{
			if (!StatusEffect) continue;
			StatusEffect->BuildResolvedEffect(OwnerAbility, ResolvedHitEffects.StatusEffects);
		}

		return ResolvedHitEffects;
	}
}
