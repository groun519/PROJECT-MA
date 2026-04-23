#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "MASkillEventSource.generated.h"

class UMASkillAbility;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillEventSource : public UObject
{
	GENERATED_BODY()

public:
	void InitializeRuntime(UMASkillAbility* SkillAbility);
	void DeinitializeRuntime();
	virtual void StartSource(UMASkillAbility* SkillAbility) { OwnerSkillAbility = SkillAbility; }
	virtual void StopSource() {}

protected:
	void EmitEvent() const;
	UMASkillAbility* GetOwnerSkillAbility() const { return OwnerSkillAbility; }

private:
	void HandleSkillActivated();
	void HandleSkillDeactivated();

protected:
	UPROPERTY(VisibleDefaultsOnly, Category="Event")
	FGameplayTag EmittedTag;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillAbility> OwnerSkillAbility;
};
