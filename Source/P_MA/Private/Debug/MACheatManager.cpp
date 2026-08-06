#include "Debug/MACheatManager.h"

#include "Engine/AssetManager.h"
#include "Framework/MAGameMode.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/MAInventoryComponent.h"
#include "Player/MAPlayerCharacter.h"

void UMACheatManager::AddCoin(const float Amount)
{
	if (AMAPlayerCharacter* PlayerCharacter = GetMAPlayerCharacter())
	{
		PlayerCharacter->Server_AddCoin(Amount);
	}
}

void UMACheatManager::RefreshShopStock()
{
	if (AMAPlayerCharacter* PlayerCharacter = GetMAPlayerCharacter())
	{
		PlayerCharacter->Server_RefreshShopStock();
	}
}

void UMACheatManager::ShopTest()
{
	if (AMAPlayerCharacter* PlayerCharacter = GetMAPlayerCharacter())
	{
		PlayerCharacter->Server_ShopTest();
	}
}

void UMACheatManager::SetMAState(const int32 NewState)
{
	if (AMAGameMode* GameMode = GetWorld()
		? GetWorld()->GetAuthGameMode<AMAGameMode>()
		: nullptr)
	{
		GameMode->SetMAState(NewState);
	}
}

void UMACheatManager::AddModule(
	const int32 ModuleId,
	const int32 Count)
{
	AMAPlayerCharacter* PlayerCharacter = GetMAPlayerCharacter();
	UMAInventoryComponent* Inventory = PlayerCharacter
		? PlayerCharacter->GetInventoryComponent()
		: nullptr;
	if (!Inventory || !Inventory->RequestAddModule(ModuleId, Count))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("AddModule failed: ModuleId=%d Count=%d"),
			ModuleId,
			Count);
	}
}

void UMACheatManager::ListItems()
{
	const AMAPlayerCharacter* PlayerCharacter = GetMAPlayerCharacter();
	const UMAInventoryComponent* Inventory = PlayerCharacter
		? PlayerCharacter->GetInventoryComponent()
		: nullptr;
	if (!Inventory) return;

	bool bFoundItem = false;
	for (int32 SlotIndex = 0; SlotIndex < Inventory->GetSlotCount(); ++SlotIndex)
	{
		const FMAInventoryEntry* Entry = Inventory->GetEntryAt(SlotIndex);
		if (!Entry || !Entry->IsItem()) continue;

		bFoundItem = true;
		UE_LOG(
			LogTemp,
			Display,
			TEXT("Slot=%d EntryId=%d ModuleId=%d Type=%s Count=%d"),
			SlotIndex,
			Entry->EntryId,
			Entry->ItemStack.Module->GetModuleId(),
			*StaticEnum<EMASkillModuleType>()->GetNameStringByValue(
				static_cast<int64>(Entry->ItemStack.Module->GetModuleType())),
			Entry->ItemStack.Count);
	}

	if (!bFoundItem)
	{
		UE_LOG(LogTemp, Display, TEXT("No inventory items."));
	}
}

void UMACheatManager::UseItem(const int32 EntryId)
{
	AMAPlayerCharacter* PlayerCharacter = GetMAPlayerCharacter();
	UMAInventoryComponent* Inventory = PlayerCharacter
		? PlayerCharacter->GetInventoryComponent()
		: nullptr;
	if (!Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("UseItem failed: EntryId=%d"), EntryId);
		return;
	}

	Inventory->UseItem(EntryId);
}

void UMACheatManager::AddSkillSubModule(
	const FString SlotTagName,
	const int32 ModuleIndex,
	const int32 SubModuleId)
{
	AMAPlayerCharacter* PlayerCharacter = GetMAPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("AddSkillSubModule must be executed by the host."));
		return;
	}

	const FGameplayTag SlotTag = FGameplayTag::RequestGameplayTag(FName(SlotTagName), false);
	const FSoftObjectPath ModulePath = UAssetManager::Get().GetPrimaryAssetPath(
		UMASkillModule::MakePrimaryAssetId(SubModuleId));
	UMASkillModule* SubModule = Cast<UMASkillModule>(ModulePath.TryLoad());
	if (!SlotTag.IsValid() || !SubModule)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("AddSkillSubModule failed: SlotTag=%s ModuleId=%d"),
			*SlotTagName,
			SubModuleId);
		return;
	}

	UMASkillManagerComponent* SkillManager = PlayerCharacter->GetSkillManagerComponent();
	if (!SkillManager || !SkillManager->AddSubModule(SlotTag, ModuleIndex, SubModule))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("AddSkillSubModule failed: SlotTag=%s ModuleIndex=%d ModuleId=%d"),
			*SlotTagName,
			ModuleIndex,
			SubModuleId);
	}
}

AMAPlayerCharacter* UMACheatManager::GetMAPlayerCharacter() const
{
	const APlayerController* PlayerController = GetPlayerController();
	return PlayerController
		? Cast<AMAPlayerCharacter>(PlayerController->GetPawn())
		: nullptr;
}
