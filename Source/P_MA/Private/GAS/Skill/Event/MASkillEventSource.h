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
	virtual void StartSource(UMASkillAbility* SkillAbility) { OwnerSkillAbility = SkillAbility; }
	virtual void StopSource() { OwnerSkillAbility = nullptr; }
	static void CreateRuntimeSources(UMASkillAbility* SkillAbility,
		const TArray<TObjectPtr<UMASkillEventSource>>& SourceTemplates,
		TArray<TObjectPtr<UMASkillEventSource>>& OutRuntimeSources);
	static void StopRuntimeSources(TArray<TObjectPtr<UMASkillEventSource>>& RuntimeSources);

	const FGameplayTag& GetEmittedTag() const { return EmittedTag; }

protected:
	void EmitEvent() const;
	UMASkillAbility* GetOwnerSkillAbility() const { return OwnerSkillAbility; }

	UPROPERTY(VisibleDefaultsOnly, Category="Event")
	FGameplayTag EmittedTag;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillAbility> OwnerSkillAbility;
};
