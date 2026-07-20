#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MASkillModuleSubsystem.generated.h"

class UMASkillModuleAsset;

DECLARE_DELEGATE_OneParam(FMASkillModuleLoaded, const UMASkillModuleAsset*);

/** Loads generated skill module assets on demand by ModuleId. */
UCLASS()
class P_MA_API UMASkillModuleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void LoadModule(int32 ModuleId, FMASkillModuleLoaded Completion);

private:
	void CompleteLoad(int32 ModuleId);

	TMap<int32, TArray<FMASkillModuleLoaded>> PendingLoads;
};
