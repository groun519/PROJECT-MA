#include "GAS/Skill/MAElementData.h"

#include "Setting/MAGameSettings.h"

const FMAElementDataRow* FMAElementDataRow::FindByTag(
	const FGameplayTag& SourceTag,
	const TCHAR* ContextString)
{
	const UDataTable* ElementalDataTable = UMAGameSettings::Get()->GetElementalDataTable();
	if (!ElementalDataTable || !SourceTag.IsValid()) return nullptr;

	FString RowNameString = SourceTag.GetTagName().ToString();
	if (!RowNameString.Split(TEXT("."), nullptr, &RowNameString, ESearchCase::CaseSensitive, ESearchDir::FromEnd)) return nullptr;

	return ElementalDataTable->FindRow<FMAElementDataRow>(FName(*RowNameString), ContextString, false);
}
