#include "GAS/Skill/MASkillSystemTypes.h"

#include "GameplayAbilitySpec.h"

int32 FMASkillSystemStatics::ResolveSlotInputID(const FGameplayTag& SlotTag)
{
	static const FGameplayTag ActiveSlotTag = FGameplayTag::RequestGameplayTag(TEXT("Skill.Slot.Active"));
	if (!SlotTag.MatchesTag(ActiveSlotTag)) return INDEX_NONE;

	static const FGameplayTag NoCooldownSlotTag = FGameplayTag::RequestGameplayTag(TEXT("Skill.Slot.Active.NoCooldown"));
	if (SlotTag.MatchesTagExact(NoCooldownSlotTag)) return 1;

	const FString SlotTagString = SlotTag.ToString();
	const FString SlotPrefix = TEXT("Skill.Slot.Active.");
	if (!SlotTagString.StartsWith(SlotPrefix)) return INDEX_NONE;

	const FString SlotIndexString = SlotTagString.RightChop(SlotPrefix.Len());
	if (SlotIndexString.Len() != 1 || !FChar::IsDigit(SlotIndexString[0])) return INDEX_NONE;

	const int32 SlotInputID = SlotIndexString[0] - TEXT('0');
	return FMath::IsWithinInclusive(SlotInputID, 1, 4) ? SlotInputID : INDEX_NONE;
}

bool FMASkillSystemStatics::IsSkillSlotTag(const FGameplayTag& Tag)
{
	return IsActiveSkillSlotTag(Tag) || IsPassiveSkillSlotTag(Tag);
}

bool FMASkillSystemStatics::IsActiveSkillSlotTag(const FGameplayTag& Tag)
{
	return ResolveSlotInputID(Tag) != INDEX_NONE;
}

bool FMASkillSystemStatics::IsPassiveSkillSlotTag(const FGameplayTag& Tag)
{
	return Tag.MatchesTagExact(GetPassiveSlotTag());
}

FGameplayTag FMASkillSystemStatics::GetPassiveSlotTag()
{
	static const FGameplayTag PassiveSlotTag = FGameplayTag::RequestGameplayTag(TEXT("Skill.Slot.Passive"));
	return PassiveSlotTag;
}

FGameplayTag FMASkillSystemStatics::ResolveCooldownTagFromSlotTag(const FGameplayTag& SlotTag)
{
	static const FGameplayTag NoCooldownSlotTag = FGameplayTag::RequestGameplayTag(TEXT("Skill.Slot.Active.NoCooldown"));
	if (SlotTag.MatchesTagExact(NoCooldownSlotTag)) return FGameplayTag();

	const int32 SlotInputID = ResolveSlotInputID(SlotTag);
	return SlotInputID == INDEX_NONE
		? FGameplayTag()
		: FGameplayTag::RequestGameplayTag(*FString::Printf(TEXT("Cooldown.Skill.Slot.%d"), SlotInputID));
}

FGameplayTag FMASkillSystemStatics::ResolveSlotTagFromAbilitySpec(const FGameplayAbilitySpec& Spec)
{
	TArray<FGameplayTag> SlotTags;
	Spec.DynamicAbilityTags.GetGameplayTagArray(SlotTags);
	for (const FGameplayTag& SlotTag : SlotTags)
	{
		if (IsSkillSlotTag(SlotTag)) return SlotTag;
	}
	return FGameplayTag();
}
