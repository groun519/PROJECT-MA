#pragma once

#include "CoreMinimal.h"
#include "NPC/MANPC.h"
#include "MAEnchanterNPC.generated.h"

class APlayerController;
class UMASkillModule;
class UMASkillModuleInstance;

UCLASS()
class P_MA_API AMAEnchanterNPC : public AMANPC
{
	GENERATED_BODY()

public:
	void EnchantModule(
		APlayerController* PlayerController,
		UMASkillModuleInstance* TargetModule,
		int32 RuneEntryId);

private:
	static bool HasExclusiveSubModuleConflict(
		const UMASkillModuleInstance& TargetModule,
		const UMASkillModule& SubModule);
	void ReportEnchantFailure(
		const UMASkillModuleInstance* TargetModule,
		int32 RuneEntryId,
		const TCHAR* Reason) const;
};
