#include "GAS/Skill/Module/MAModuleQualityData.h"

int32 UMAModuleQualityData::ResolvePrice(const FMAModuleQuality& Quality) const
{
	const FMAModuleGradeData* FoundGradeData = GradeData.Find(Quality.Grade);
	const FMAModuleRarityData* FoundRarityData = RarityData.Find(Quality.Rarity);
	if (!FoundGradeData || !FoundRarityData) return 0;

	const int32 Price = FoundGradeData->BasePrice + FoundRarityData->PriceOffset;
	return FMath::Max(0, Price);
}
