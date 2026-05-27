#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Payload/MASkillPayloadEntry.h"
#include "UObject/Object.h"
#include "MASkillPayloadWriter.generated.h"

class UMASkillAbility;
class UMASkillModuleInstance;
struct FGameplayEventData;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillPayloadWriter : public UObject
{
	GENERATED_BODY()

public:
	virtual void WritePayload(UMASkillAbility&, const FGameplayEventData&, UMASkillModuleInstance*) const {}
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Static Payload Writer")
class P_MA_API UMASkillPayloadWriter_Static : public UMASkillPayloadWriter
{
	GENERATED_BODY()

public:
	virtual void WritePayload(UMASkillAbility& SkillAbility, const FGameplayEventData& EventData, UMASkillModuleInstance* EventScope) const override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Payload")
	TArray<FMASkillPayloadEntry> Payloads;
};
