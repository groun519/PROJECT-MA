// Fill out your copyright notice in the Description page of Project Settings.

#include "MAComboAbility.h"
#include "GamePlayTagsManager.h"

UMAComboAbility::UMAComboAbility()
{
    AbilityTags.AppendTags(AbilityTagsEditable);
    BlockAbilitiesWithTag.AppendTags(BlockAbilitiesWithTagEditable);
    CancelAbilitiesWithTag.AppendTags(CancelAbilitiesWithTagEditable);
    ActivationOwnedTags.AppendTags(ActivationOwnedTagsEditable);
    ActivationRequiredTags.AppendTags(ActivationRequiredTagsEditable);
    ActivationBlockedTags.AppendTags(ActivationBlockedTagsEditable);
    SourceRequiredTags.AppendTags(SourceRequiredTagsEditable);
    SourceBlockedTags.AppendTags(SourceBlockedTagsEditable);
    TargetRequiredTags.AppendTags(TargetRequiredTagsEditable);
    TargetBlockedTags.AppendTags(TargetBlockedTagsEditable);

    FGameplayTag BasicAttackTag = FGameplayTag::RequestGameplayTag(FName("Ability.BasicAttack"));

    AbilityTags.AddTag(BasicAttackTag);
    BlockAbilitiesWithTag.AddTag(BasicAttackTag);
}
