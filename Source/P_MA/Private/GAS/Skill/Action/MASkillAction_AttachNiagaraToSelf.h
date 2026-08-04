#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "MASkillAction_AttachNiagaraToSelf.generated.h"

class UNiagaraSystem;

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Attach Niagara To Self")
class P_MA_API UMASkillAction_AttachNiagaraToSelf : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_AttachNiagaraToSelf() { SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub; }

	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Niagara")
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Niagara")
	FName SocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, Category="Niagara", meta=(ClampMin="0.0"))
	float LifeSpan = 0.f;
};
