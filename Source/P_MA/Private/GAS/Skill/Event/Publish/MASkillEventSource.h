#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "MASkillEventSource.generated.h"

class UMASkillAbility;
class UMASkillModuleInstance;
class UMASkillPayloadWriter;
struct FGameplayEventData;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillEventSource : public UObject
{
	GENERATED_BODY()

public:
	void InitializeRuntime(UMASkillAbility* SkillAbility, UMASkillModuleInstance* EventOwnerScope);
	void DeinitializeRuntime();
	virtual void StartSource(UMASkillAbility* SkillAbility) { OwnerSkillAbility = SkillAbility; }
	virtual void StopSource() {}
	virtual void HandleSourceEvent(
		UMASkillAbility& SkillAbility,
		UMASkillModuleInstance& InEventOwnerScope,
		const FGameplayTag& SourceEventTag,
		const FGameplayEventData& EventData) const;
	void SetRuntimeScope(UMASkillModuleInstance* InRuntimeScope) { RuntimeScope = InRuntimeScope; }
	UMASkillModuleInstance* GetRuntimeScope() const { return RuntimeScope; }

protected:
	void EmitEvent() const;
	void EmitEvent(const FGameplayEventData& EventData) const;
	void EmitEvent(UMASkillAbility& SkillAbility, UMASkillModuleInstance& InEventOwnerScope, const FGameplayEventData& EventData) const;
	UMASkillAbility* GetOwnerSkillAbility() const { return OwnerSkillAbility; }

private:
	void HandleScopedEvent(const FGameplayTag& SourceEventTag, const FGameplayEventData& EventData);
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
	TObjectPtr<UMASkillModuleInstance> RuntimeScope = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillModuleInstance> EventOwnerScope = nullptr;

	FDelegateHandle ScopedEventDelegateHandle;
};

