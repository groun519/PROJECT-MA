#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "MASkillAction_LinkedGameplayEffect.generated.h"

class UGameplayEffect;
struct FMASkillPayloadAccessor;

USTRUCT(BlueprintType)
struct P_MA_API FMASkillLinkedGameplayEffectSetByCaller
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="SetByCaller", meta=(Categories="Data"))
	FGameplayTag SetByCallerTag;

	UPROPERTY(EditDefaultsOnly, Category="SetByCaller")
	float BaseValue = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="SetByCaller", meta=(Categories="Data"))
	FGameplayTag PayloadTag;

	UPROPERTY(EditDefaultsOnly, Category="SetByCaller")
	float PayloadMultiplier = 1.f;

	bool TryResolve(const FMASkillPayloadAccessor& Payloads, float& OutValue) const;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Link Module Gameplay Effect")
class P_MA_API UMASkillAction_LinkModuleGameplayEffect : public UMASkillAction
{
	GENERATED_BODY()

public:
	virtual void Execute(UMASkillAbility& OwnerAbility, const FMASkillEvent& Event, const FMASkillScopes& Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="GameplayEffect")
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	UPROPERTY(EditDefaultsOnly, Category="GameplayEffect", meta=(ClampMin="0.0"))
	float EffectLevel = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="GameplayEffect", meta=(Categories="Data"))
	FGameplayTag HandlePayloadTag;

	UPROPERTY(EditDefaultsOnly, Category="GameplayEffect")
	TArray<FMASkillLinkedGameplayEffectSetByCaller> SetByCallers;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Modify Linked Gameplay Effect")
class P_MA_API UMASkillAction_ModifyLinkedGameplayEffect : public UMASkillAction
{
	GENERATED_BODY()

public:
	virtual void Execute(UMASkillAbility& OwnerAbility, const FMASkillEvent& Event, const FMASkillScopes& Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="GameplayEffect", meta=(Categories="Data"))
	FGameplayTag HandlePayloadTag;

	UPROPERTY(EditDefaultsOnly, Category="GameplayEffect")
	TArray<FMASkillLinkedGameplayEffectSetByCaller> SetByCallers;
};
