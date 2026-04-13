#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GAS/Skill/Event/MASkillGameplayEventPart.h"
#include "GameplayTagContainer.h"
#include "MASkillDefinition.generated.h"

class UMASkillEventSource;
class UMASkillFlowPart;
class UDataTable;

UCLASS(BlueprintType)
class P_MA_API UMASkillDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	const FGameplayTag& GetElementalTag() const { return ElementalTag; }
	const TArray<TObjectPtr<UMASkillEventSource>>& GetEventSources() const { return EventSources; }
	const TArray<FMASkillGameplayEventPart>& GetEventParts() const { return EventParts; }
	const TArray<TObjectPtr<UMASkillFlowPart>>& GetFlowParts() const { return FlowParts; }

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
};
