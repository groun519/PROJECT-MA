#include "NPC/MAEnchanterNPC.h"

#include "Convenience/MAInteractableComponent.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/MAInventoryComponent.h"
#include "Player/MAPlayerCharacter.h"
#include "Setting/MAGameSettings.h"

void AMAEnchanterNPC::EnchantModule(
	APlayerController* PlayerController,
	UMASkillModuleInstance* TargetModule,
	const int32 RuneEntryId)
{
	AMAPlayerCharacter* PlayerCharacter = PlayerController
		? Cast<AMAPlayerCharacter>(PlayerController->GetPawn())
		: nullptr;
	UMAInventoryComponent* Inventory = PlayerCharacter
		? PlayerCharacter->GetInventoryComponent()
		: nullptr;
	if (!HasAuthority()
		|| !PlayerCharacter
		|| !Inventory
		|| !InteractableComponent->CanServerInteract(PlayerCharacter))
	{
		return ReportEnchantFailure(TargetModule, RuneEntryId, TEXT("Unavailable"));
	}

	if (!TargetModule || !TargetModule->IsValid())
	{
		return ReportEnchantFailure(TargetModule, RuneEntryId, TEXT("InvalidTarget"));
	}
	if (TargetModule->GetTypedOuter<AActor>() != PlayerCharacter
		|| !TargetModule->IsInSkillSlot())
	{
		return ReportEnchantFailure(TargetModule, RuneEntryId, TEXT("TargetNotOwned"));
	}

	const FMAInventoryEntry* RuneEntry = Inventory->FindEntry(RuneEntryId);
	const FMAInventoryStack* RuneStack = RuneEntry ? RuneEntry->GetStack() : nullptr;
	UMASkillModule* RuneModule = RuneStack ? RuneStack->Module.Get() : nullptr;
	if (!RuneModule || RuneModule->GetModuleType() != EMASkillModuleType::Sub)
	{
		return ReportEnchantFailure(TargetModule, RuneEntryId, TEXT("InvalidRune"));
	}

	const TArray<TObjectPtr<UMASkillModule>>& SubModules = TargetModule->GetModuleGroup().SubModules;
	if (SubModules.Num() >= FMath::Max(0, UMAGameSettings::Get()->MaxEnchantmentsPerModule))
	{
		return ReportEnchantFailure(TargetModule, RuneEntryId, TEXT("LimitReached"));
	}
	if (HasExclusiveSubModuleConflict(*TargetModule, *RuneModule))
	{
		return ReportEnchantFailure(TargetModule, RuneEntryId, TEXT("ExclusiveConflict"));
	}
	const int32 RemainingRuneCount = RuneStack->Count - 1;
	if (!TargetModule->SetSubModuleAt(SubModules.Num(), RuneModule))
	{
		return ReportEnchantFailure(TargetModule, RuneEntryId, TEXT("InvalidTarget"));
	}

	verify(Inventory->SetStackCount(RuneEntryId, RemainingRuneCount));
}

bool AMAEnchanterNPC::HasExclusiveSubModuleConflict(
	const UMASkillModuleInstance& TargetModule,
	const UMASkillModule& SubModule)
{
	const FGameplayTag ExclusiveTag = FGameplayTag::RequestGameplayTag(TEXT("Module.Exclusive"), false);
	const FGameplayTag UniqueTag = FGameplayTag::RequestGameplayTag(TEXT("Module.Exclusive.Unique"), false);

	for (const FGameplayTag& ModuleTag : SubModule.GetModuleTags())
	{
		if (!ModuleTag.MatchesTag(ExclusiveTag)) continue;

		for (const UMASkillModule* ExistingSubModule : TargetModule.GetModuleGroup().SubModules)
		{
			if (!ExistingSubModule) continue;
			if (ModuleTag.MatchesTag(UniqueTag))
			{
				if (ExistingSubModule == &SubModule) return true;
			}
			else if (ExistingSubModule->GetModuleTags().HasTagExact(ModuleTag))
			{
				return true;
			}
		}
	}
	return false;
}

void AMAEnchanterNPC::ReportEnchantFailure(
	const UMASkillModuleInstance* TargetModule,
	const int32 RuneEntryId,
	const TCHAR* Reason) const
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("EnchantModule failed: Enchanter=%s Target=%s RuneEntryId=%d Result=%s"),
		*GetName(),
		*GetNameSafe(TargetModule),
		RuneEntryId,
		Reason);
}
