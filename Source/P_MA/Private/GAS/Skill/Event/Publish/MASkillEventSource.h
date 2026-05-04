#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "MASkillEventSource.generated.h"

class UMASkillAbility;
class UMASkillPayloadWriter;
class UMASkillRuntimeScope;
struct FGameplayEventData;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillEventSource : public UObject
{
	GENERATED_BODY()

public:
	void InitializeRuntime(UMASkillAbility* SkillAbility);
	void DeinitializeRuntime();
	virtual void StartSource(UMASkillAbility* SkillAbility) { OwnerSkillAbility = SkillAbility; }
	virtual void StopSource() {}
	void SetRuntimeScope(UMASkillRuntimeScope* InRuntimeScope) { RuntimeScope = InRuntimeScope; }
	UMASkillRuntimeScope* GetRuntimeScope() const { return RuntimeScope; }

protected:
	void EmitEvent() const;
	void EmitEvent(const FGameplayEventData& Payload) const;
	UMASkillAbility* GetOwnerSkillAbility() const { return OwnerSkillAbility; }

private:
	void HandleSkillActivated();
	void HandleSkillDeactivated();

protected:
	UPROPERTY(VisibleDefaultsOnly, Category="Event")
	FGameplayTag EmittedTag;

	UPROPERTY(EditDefaultsOnly, Instanced, Category="Payload", meta=(DisplayName="Pre-Emit Payload Writers"))
	TArray<TObjectPtr<UMASkillPayloadWriter>> PreEmitPayloadWriters;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillAbility> OwnerSkillAbility;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillRuntimeScope> RuntimeScope = nullptr;
};

