#include "Debug/MACheatManager.h"

#include "Engine/AssetManager.h"
#include "Framework/MAGameMode.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GameFramework/PlayerController.h"
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
