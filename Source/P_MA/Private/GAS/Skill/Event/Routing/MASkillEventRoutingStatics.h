#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/MASkillEventTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MASkillEventRoutingStatics.generated.h"

class UMASkillAbility;
class UMASkillManagerComponent;
class UMASkillModuleInstance;

UCLASS()
class P_MA_API UMASkillEventRoutingStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static bool TryNotifySkillEvent(
		UMASkillAbility* ExecutorAbility,
		FMASkillEvent Event,
		UMASkillModuleInstance* ModuleScope = nullptr);

	static bool TryNotifySkillEvent(
		UMASkillAbility* ExecutorAbility,
		FGameplayTag EventTag,
		const FMASkillScopes& SourceScopes);

	static bool TryNotifyGlobalEvent(
		UMASkillManagerComponent* SkillManager,
		FMASkillEvent Event);

private:
	static bool TryRoute(
		UMASkillManagerComponent* SkillManager,
		FMASkillEvent Event,
		UMASkillAbility* ExecutorAbility);
};
