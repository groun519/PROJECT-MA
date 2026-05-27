#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MASkillAction.generated.h"

struct FGameplayEventData;
class UMASkillAbility;
class UMASkillModuleInstance;

struct FMASkillEventScopes
{
	UMASkillModuleInstance* BindingScope = nullptr;
	UMASkillModuleInstance* EventScope = nullptr;
};

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillAction : public UObject
{
	GENERATED_BODY()

public:
	virtual void Execute(
		UMASkillAbility& OwnerAbility,
		const FGameplayEventData& EventData,
		const FMASkillEventScopes& Scopes)
		PURE_VIRTUAL(UMASkillAction::Execute, );
};
