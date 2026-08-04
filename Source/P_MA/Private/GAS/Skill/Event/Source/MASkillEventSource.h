#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "MASkillEventSource.generated.h"

class UMASkillManagerComponent;
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillEventSource : public UObject
{
	GENERATED_BODY()

public:
	void InitializeRuntime(UMASkillManagerComponent* SkillManager);
	void DeinitializeRuntime();
	virtual bool RequiresRuntimeInstance() const { return false; }
	virtual bool HasSameRuntimeConfiguration(const UMASkillEventSource& Other) const;
	const FGameplayTag& GetEmittedTag() const { return EmittedTag; }

protected:
	virtual void StartSource() {}
	virtual void StopSource() {}
	UMASkillManagerComponent* GetSkillManager() const { return OwnerSkillManager; }

	UPROPERTY(VisibleDefaultsOnly, Category="Event")
	FGameplayTag EmittedTag;

private:
	UPROPERTY(Transient)
	TObjectPtr<UMASkillManagerComponent> OwnerSkillManager;
};
