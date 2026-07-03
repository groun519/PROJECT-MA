#include "GAS/Skill/Module/MAModuleQualityData.h"

int32 UMAModuleQualityData::ResolvePrice(const FMAModuleQuality& Quality) const
{
	const FMAModuleRarityData* FoundRarityData = RarityData.Find(Quality.Rarity);
	check(FoundRarityData);

	return FMath::Max(0, FoundRarityData->BasePrice);
}

const FMAModuleTypeData* UMAModuleQualityData::FindVisualData(
	const FGameplayTagContainer& ModuleVisualTags) const
{
	for (const FGameplayTag& VisualTag : ModuleVisualTags)
	{
		if (const FMAModuleTypeData* FoundVisualData = VisualData.Find(VisualTag))
		{
			return FoundVisualData;
		}
	}

	return nullptr;
}
