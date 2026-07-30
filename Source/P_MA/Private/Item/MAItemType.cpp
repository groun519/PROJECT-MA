#include "Item/MAItemType.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Item/Data/MAItemData.h"

bool FMAItemId::IsValid() const
{
	return GetItemType() != nullptr && !RowName.IsNone();
}

const UMAItemType* FMAItemId::GetItemType() const
{
	return Type ? Type->GetDefaultObject<UMAItemType>() : nullptr;
}

bool FMAItemId::operator==(const FMAItemId& Other) const
{
	return Type == Other.Type && RowName == Other.RowName;
}

const FMAItemDataRow* UMAItemType::FindItemData(const FName RowName) const
{
	const UDataTable* DataTable = LoadItemDataTable();
	return DataTable && !RowName.IsNone()
		? DataTable->FindRow<FMAItemDataRow>(RowName, TEXT("ItemData"), false)
		: nullptr;
}

const UDataTable* UMARuneItemType::LoadItemDataTable() const
{
	return ItemDataTable.LoadSynchronous();
}

const UDataTable* UMAConsumableItemType::LoadItemDataTable() const
{
	return ItemDataTable.LoadSynchronous();
}

EMAItemUseResult UMAConsumableItemType::TryUse(
	AActor& OwnerActor,
	const FName RowName) const
{
	const UDataTable* DataTable = LoadItemDataTable();
	const FMAConsumableItemDataRow* ItemData = DataTable
		? DataTable->FindRow<FMAConsumableItemDataRow>(RowName, TEXT("UseItem"), false)
		: nullptr;
	if (!ItemData) return EMAItemUseResult::InvalidData;

	const TSubclassOf<UGameplayEffect> EffectClass = ItemData->UseEffect.LoadSynchronous();
	UAbilitySystemComponent* AbilitySystemComponent =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(&OwnerActor);
	if (!EffectClass || !AbilitySystemComponent) return EMAItemUseResult::InvalidData;

	const FGameplayEffectSpecHandle EffectSpec = AbilitySystemComponent->MakeOutgoingSpec(
		EffectClass,
		1.f,
		AbilitySystemComponent->MakeEffectContext());
	if (!EffectSpec.IsValid()) return EMAItemUseResult::Failed;

	return AbilitySystemComponent
		->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get())
		.WasSuccessfullyApplied()
		? EMAItemUseResult::Success
		: EMAItemUseResult::Failed;
}
