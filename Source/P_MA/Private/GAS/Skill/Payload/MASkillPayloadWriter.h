#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Payload/MASkillPayloadEntry.h"
#include "UObject/Object.h"
#include "MASkillPayloadWriter.generated.h"

class UMASkillAbility;
struct FGameplayEventData;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillPayloadWriter : public UObject
{
	GENERATED_BODY()

public:
	virtual void WritePayload(UMASkillAbility&, const FGameplayEventData&) const {}
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Static Payload Writer")
class P_MA_API UMASkillPayloadWriter_Static : public UMASkillPayloadWriter
{
	GENERATED_BODY()

public:
	virtual void WritePayload(UMASkillAbility& SkillAbility, const FGameplayEventData& EventData) const override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Payload")
	TArray<FMASkillPayloadEntry> Payloads;
};
