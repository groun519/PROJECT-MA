#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GAS/Skill/Event/MASkillGameplayEventPart.h"
#include "MASkillDefinition.generated.h"

class UAnimMontage;
class UGameplayEffect;
class UMASkillFlowPart;

UCLASS(BlueprintType)
class P_MA_API UMASkillDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	UAnimMontage* GetSkillMontage() const { return SkillMontage; }
	TSubclassOf<UGameplayEffect> GetDefaultDamageEffect() const { return DefaultDamageEffect; }
	const TArray<FMASkillGameplayEventPart>& GetEventParts() const { return EventParts; }
	UMASkillFlowPart* GetFlowPart() const { return FlowPart; }

private:
	/** Animation **/
	UPROPERTY(EditDefaultsOnly, Category="Animation")
	TObjectPtr<UAnimMontage> SkillMontage;

	/** Damage **/
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	TSubclassOf<UGameplayEffect> DefaultDamageEffect;

	/** Flow **/
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Flow")
	TObjectPtr<UMASkillFlowPart> FlowPart;

	/** Event **/
	UPROPERTY(EditDefaultsOnly, Category="Event")
	TArray<FMASkillGameplayEventPart> EventParts;
};
