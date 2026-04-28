#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GAS/Skill/Event/Binding/MASkillGameplayEventBinding.h"
#include "GAS/Skill/Payload/MASkillPayloadEntry.h"
#include "GAS/Skill/Step/MASkillStep.h"
#include "GameplayTagContainer.h"
#include "MASkillDefinition.generated.h"

class UMASkillEventSource;
class UMASkillRuntimeScope;
struct FMASkillPayloadStore;
struct FMASkillAssembler;

UCLASS(BlueprintType)
class P_MA_API UMASkillDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	const FGameplayTag& GetElementalTag() const { return ElementalTag; }
	const TArray<TObjectPtr<UMASkillStep>>& GetSkillSteps() const { return SkillSteps; }
	const TArray<FMASkillGameplayEventBinding>& GetEventBindings() const { return EventBindings; }
	const TArray<TObjectPtr<UMASkillEventSource>>& GetEventSources() const { return EventSources; }
	bool IsRuntimeAssembledDefinition() const { return bIsRuntimeAssembledDefinition; }

	void ApplyPayloadsTo(FMASkillPayloadStore& PayloadStore) const
	{
		for (const FMASkillPayloadEntry& PayloadEntry : Payloads)
		{
			PayloadEntry.ApplyTo(PayloadStore);
		}
	}

private:
	void ResetAssemblyData();
	void AppendFrom(const UMASkillDefinition& Other);

	friend struct FMASkillAssembler;

	UPROPERTY(EditDefaultsOnly, Category="Elemental", meta=(Categories="Elemental"))
	FGameplayTag ElementalTag;

	/** Preferred step pipeline. Each step owns its own montage and runtime logic. **/
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Step")
	TArray<TObjectPtr<UMASkillStep>> SkillSteps;

	/** Event Source **/
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Event")
	TArray<TObjectPtr<UMASkillEventSource>> EventSources;

	/** Event **/
	UPROPERTY(EditDefaultsOnly, Category="Event", meta=(DisplayName="Event Bindings"))
	TArray<FMASkillGameplayEventBinding> EventBindings;

	UPROPERTY(EditDefaultsOnly, Category="Payload")
	TArray<FMASkillPayloadEntry> Payloads;

	UPROPERTY(Transient)
	bool bIsRuntimeAssembledDefinition = false;
};
