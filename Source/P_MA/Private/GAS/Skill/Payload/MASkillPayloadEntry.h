#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "GAS/Skill/Payload/MASkillPayloadValueType.h"
#include "MASkillPayloadEntry.generated.h"

struct FMASkillPayloadStore;

USTRUCT(BlueprintType)
struct P_MA_API FMASkillPayloadEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Payload")
	FGameplayTag PayloadTag;

	UPROPERTY(EditDefaultsOnly, Category="Payload")
	EMASkillPayloadValueType ValueType = EMASkillPayloadValueType::Scalar;

	UPROPERTY(EditDefaultsOnly, Category="Payload", meta=(EditCondition="ValueType == EMASkillPayloadValueType::Scalar", EditConditionHides))
	float ScalarValue = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Payload", meta=(EditCondition="ValueType == EMASkillPayloadValueType::Vector", EditConditionHides))
	FVector VectorValue = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category="Payload", meta=(EditCondition="ValueType == EMASkillPayloadValueType::Object", EditConditionHides))
	TObjectPtr<UObject> ObjectValue = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Payload", meta=(EditCondition="ValueType == EMASkillPayloadValueType::Struct", EditConditionHides, BaseStruct="/Script/P_MA.MASkillPayloadStructBase"))
	FInstancedStruct StructValue;

	void ApplyTo(FMASkillPayloadStore& PayloadStore) const;
};

