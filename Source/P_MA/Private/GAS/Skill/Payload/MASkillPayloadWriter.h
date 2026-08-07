#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"

struct P_MA_API FMASkillPayloadWriter
{
	FMASkillPayloadWriter() = default;
	FMASkillPayloadWriter(FMASkillPayloadStore* InSkillPayloads, FMASkillPayloadStore* InModulePayloads);

	bool IsValid() const { return SkillPayloads || ModulePayloads; }

	void SetScalar(EMASkillPayloadScope Scope, const FGameplayTag& Key, float Value);
	bool AddScalar(
		EMASkillPayloadScope Scope,
		const FGameplayTag& Key,
		float Value,
		TOptional<float> MinimumValue = {});
	bool MultiplyScalar(EMASkillPayloadScope Scope, const FGameplayTag& Key, float Value);
	void SetVector(EMASkillPayloadScope Scope, const FGameplayTag& Key, const FVector& Value);
	void SetObject(EMASkillPayloadScope Scope, const FGameplayTag& Key, UObject* Value);

	template <typename StructType>
	void SetStruct(EMASkillPayloadScope Scope, const FGameplayTag& Key, const StructType& Value)
	{
		if (FMASkillPayloadStore* Store = GetStore(Scope)) Store->SetStruct(Key, Value);
	}

private:
	FMASkillPayloadStore* GetStore(EMASkillPayloadScope Scope) const;

	FMASkillPayloadStore* SkillPayloads = nullptr;
	FMASkillPayloadStore* ModulePayloads = nullptr;
};
