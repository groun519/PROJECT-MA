#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "MASkillEventSource.generated.h"

class UMASkillAbility;
struct FSkillRuntimeContext;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillEventSource : public UObject
{
	GENERATED_BODY()

public:
	virtual void StartSource(UMASkillAbility* SkillAbility, FSkillRuntimeContext* InRuntimeContext)
	{
		OwnerSkillAbility = SkillAbility;
		RuntimeContext = InRuntimeContext;
	}

	virtual void StopSource()
	{
		OwnerSkillAbility = nullptr;
		RuntimeContext = nullptr;
	}

	const FGameplayTag& GetEmittedTag() const { return EmittedTag; }

protected:
	void EmitEvent() const;
	UMASkillAbility* GetOwnerSkillAbility() const { return OwnerSkillAbility; }
	FSkillRuntimeContext* GetRuntimeContext() const { return RuntimeContext; }

	UPROPERTY(VisibleDefaultsOnly, Category="Event")
	FGameplayTag EmittedTag;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillAbility> OwnerSkillAbility;

	FSkillRuntimeContext* RuntimeContext = nullptr;
};
