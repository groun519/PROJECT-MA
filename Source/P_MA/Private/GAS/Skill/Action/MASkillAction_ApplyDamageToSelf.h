#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GameplayTagContainer.h"
#include "MASkillAction_ApplyDamageToSelf.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Apply Damage To Self")
class P_MA_API UMASkillAction_ApplyDamageToSelf : public UMASkillAction
{
	GENERATED_BODY()

public:
	virtual void Execute(UMASkillAbility& OwnerAbility, const FGameplayEventData& EventData, const FMASkillEventScopes& Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(Categories="Damage"))
	FGameplayTag DamagePayloadTag;
};
