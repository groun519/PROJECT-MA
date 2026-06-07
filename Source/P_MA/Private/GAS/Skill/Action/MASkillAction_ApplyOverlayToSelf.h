#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "MASkillAction_ApplyOverlayToSelf.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Apply Overlay To Self")
class P_MA_API UMASkillAction_ApplyOverlayToSelf : public UMASkillAction
{
	GENERATED_BODY()

public:
	virtual void Execute(UMASkillAbility& OwnerAbility, const FGameplayEventData& EventData, const FMASkillEventScopes& Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Overlay")
	FLinearColor BaseColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category="Overlay", meta=(ClampMin="0.0", ClampMax="1.0"))
	float Alpha = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Overlay", meta=(ClampMin="0.0"))
	float Duration = 0.2f;
};
