// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Modules/SkillModule_Melee.h"

#include "GAS/Ability/MAGameplayAbility_Skill.h"

void USkillModule_Melee::OnAbilityActivated()
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	if (!Skill)	return;

	float PlayRate = Skill->GetTotalAnimSpeed();
	
}

void USkillModule_Melee::OnAbilityEnded(bool bWasCancelled)
{
	
}

void USkillModule_Melee::OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData& Payload)
{
	if (EventTag.MatchesTag(DamageEventTag))
	{
		PerformMeleeTrace(Payload);
	}
}

void USkillModule_Melee::PerformMeleeTrace(const FGameplayEventData& Payload)
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	if (!Skill)	return;

	TArray<FHitResult> HitResults = Skill->GetHitResultFromVirtualSocketTargetData(Payload.TargetData);
	Skill->ApplyDamageToHitResults(HitResults);
}
