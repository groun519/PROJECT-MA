#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GAS/Skill/Event/MASkillGameplayEventPart.h"
#include "MASkillDefinition.generated.h"

class UGameplayEffect;
class UMASkillEventSource;
class UMASkillFlowPart;

UCLASS(BlueprintType)
class P_MA_API UMASkillDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	TSubclassOf<UGameplayEffect> GetDefaultDamageEffect() const { return DefaultDamageEffect; }
	const TArray<TObjectPtr<UMASkillEventSource>>& GetEventSources() const { return EventSources; }
	const TArray<FMASkillGameplayEventPart>& GetEventParts() const { return EventParts; }
	const TArray<TObjectPtr<UMASkillFlowPart>>& GetFlowParts() const { return FlowParts; }

private:
	/** Damage **/
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	TSubclassOf<UGameplayEffect> DefaultDamageEffect;

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
