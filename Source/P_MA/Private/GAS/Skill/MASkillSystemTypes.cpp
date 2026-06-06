#include "GAS/Skill/MASkillSystemTypes.h"

#include "GameplayAbilitySpec.h"

int32 FMASkillSystemStatics::ResolveSlotInputID(const FGameplayTag& SlotTag)
{
	const FString SlotTagString = SlotTag.ToString();
	const FString SlotPrefix = TEXT("Skill.Slot.");
	if (!SlotTagString.StartsWith(SlotPrefix)) return INDEX_NONE;

	const FString SlotIndexString = SlotTagString.RightChop(SlotPrefix.Len());
	if (SlotIndexString.Len() != 1 || !FChar::IsDigit(SlotIndexString[0])) return INDEX_NONE;

	const int32 SlotInputID = SlotIndexString[0] - TEXT('0');
	return FMath::IsWithinInclusive(SlotInputID, 1, 4) ? SlotInputID : INDEX_NONE;
}

bool FMASkillSystemStatics::IsSkillSlotTag(const FGameplayTag& Tag)
{
	return ResolveSlotInputID(Tag) != INDEX_NONE;
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
