#include "Debug/MACheatManager.h"

#include "EngineUtils.h"
#include "Framework/MAGameMode.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/MAInventoryComponent.h"
#include "NPC/MAEnchanterNPC.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/MAPlayerController.h"

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
	const TArray<FMAInventoryEntry>& Entries = Inventory->GetEntries();
	for (int32 SlotIndex = 0; SlotIndex < Entries.Num(); ++SlotIndex)
	{
		const FMAInventoryEntry& Entry = Entries[SlotIndex];
		const FMAInventoryStack* Stack = Entry.GetStack();
		if (!Stack) continue;

		bFoundItem = true;
		UE_LOG(
			LogTemp,
			Display,
			TEXT("Slot=%d EntryId=%d ModuleId=%d Type=%s Count=%d"),
			SlotIndex,
			Entry.EntryId,
			Stack->Module->GetModuleId(),
			*StaticEnum<EMASkillModuleType>()->GetNameStringByValue(
				static_cast<int64>(Stack->Module->GetModuleType())),
			Stack->Count);
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

void UMACheatManager::EnchantModule(
	const FString SlotTagName,
	const int32 ModuleIndex,
	const int32 RuneEntryId)
{
	AMAPlayerCharacter* PlayerCharacter = GetMAPlayerCharacter();
	if (!PlayerCharacter) return;

	const FGameplayTag SlotTag = FGameplayTag::RequestGameplayTag(FName(SlotTagName), false);
	UMASkillManagerComponent* SkillManager = PlayerCharacter->GetSkillManagerComponent();
	UMASkillModuleInstance* ModuleInstance = SkillManager
		? SkillManager->GetModuleInstanceAt(SlotTag, ModuleIndex)
		: nullptr;
	AMAEnchanterNPC* EnchanterNPC = nullptr;
	for (TActorIterator<AMAEnchanterNPC> It(GetWorld()); It; ++It)
	{
		EnchanterNPC = *It;
		break;
	}

	AMAPlayerController* PlayerController = Cast<AMAPlayerController>(GetPlayerController());
	if (ModuleInstance && EnchanterNPC && PlayerController)
	{
		PlayerController->RequestEnchantModule(EnchanterNPC, ModuleInstance, RuneEntryId);
		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("EnchantModule failed: SlotTag=%s ModuleIndex=%d RuneEntryId=%d"),
		*SlotTagName,
		ModuleIndex,
		RuneEntryId);
}

AMAPlayerCharacter* UMACheatManager::GetMAPlayerCharacter() const
{
	const APlayerController* PlayerController = GetPlayerController();
	return PlayerController
		? Cast<AMAPlayerCharacter>(PlayerController->GetPawn())
		: nullptr;
}
