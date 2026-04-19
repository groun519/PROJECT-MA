#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GAS/Skill/Event/MASkillGameplayEventPart.h"
#include "GAS/Skill/Payload/MASkillPayloadEntry.h"
#include "GameplayTagContainer.h"
#include "MASkillDefinition.generated.h"

class UMASkillEventSource;
class UMASkillFlowPart;
class UDataTable;
struct FMASkillPayloadStore;
class UMASkillAction;

UCLASS(BlueprintType)
class P_MA_API UMASkillDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	const FGameplayTag& GetElementalTag() const { return ElementalTag; }
	const TArray<TObjectPtr<UMASkillEventSource>>& GetEventSources() const { return EventSources; }
	const TArray<FMASkillGameplayEventPart>& GetEventParts() const { return EventParts; }
	const TArray<TObjectPtr<UMASkillFlowPart>>& GetFlowParts() const { return FlowParts; }
	const TArray<FMASkillPayloadEntry>& GetPayloads() const { return Payloads; }

	void ApplyPayloadsTo(FMASkillPayloadStore& PayloadStore) const
	{
		for (const FMASkillPayloadEntry& PayloadEntry : Payloads)
		{
			PayloadEntry.ApplyTo(PayloadStore);
		}
	}

	void CollectEventActions(TSet<FGameplayTag>& RequiredEventTags, TMap<FGameplayTag, TArray<TObjectPtr<UMASkillAction>>>& ActionsByEvent) const
	{
		for (const FMASkillGameplayEventPart& EventPart : EventParts)
		{
			EventPart.ContributeTo(RequiredEventTags, ActionsByEvent);
		}
	}

private:
	UPROPERTY(EditDefaultsOnly, Category="Elemental", meta=(Categories="Elemental"))
	FGameplayTag ElementalTag;

	/** Preferred flow pipeline. Each flow owns its own montage and runtime logic. **/
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Flow")
	TArray<TObjectPtr<UMASkillFlowPart>> FlowParts;

	/** Event Source **/
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Event")
	TArray<TObjectPtr<UMASkillEventSource>> EventSources;

	/** Event **/
	UPROPERTY(EditDefaultsOnly, Category="Event")
	TArray<FMASkillGameplayEventPart> EventParts;

	UPROPERTY(EditDefaultsOnly, Category="Payload")
	TArray<FMASkillPayloadEntry> Payloads;
};
