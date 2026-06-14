#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/MASkillEventTypes.h"
#include "UObject/Object.h"
#include "MASkillEventRouter.generated.h"

class UMASkillAbility;
class UMASkillEventSource;
class UMASkillManagerComponent;
struct FMASkillSlotRuntimeState;

UCLASS()
class P_MA_API UMASkillEventRouter : public UObject
{
	GENERATED_BODY()

public:
	bool TryRoute(FMASkillEvent Event, UMASkillAbility* ExecutorAbility);
	void Refresh(const TArray<FMASkillSlotRuntimeState>& SkillSlotRuntimeStates);
	void Clear();

private:
	UMASkillManagerComponent* GetSkillManager() const;

	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UMASkillEventSource>> Routes;
};
