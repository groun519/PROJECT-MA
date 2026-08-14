#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"

struct P_MA_API FMASkillPayloadWriter
{
	FMASkillPayloadWriter() = default;
	FMASkillPayloadWriter(FMASkillPayloadStore* InSkillPayloads, FMASkillPayloadStore* InModulePayloads);

	bool IsValid() const { return SkillPayloads || ModulePayloads; }

	void SetScalar(
		EMASkillPayloadScope Scope,
		const FGameplayTag& Key,
		float Value,
		bool bKeepValueOnPayloadReset = false);
	bool AddScalar(
		EMASkillPayloadScope Scope,
		const FGameplayTag& Key,
		float Value,
		TOptional<float> MinimumValue = {});
	bool MultiplyScalar(EMASkillPayloadScope Scope, const FGameplayTag& Key, float Value);
	void SetVector(
		EMASkillPayloadScope Scope,
		const FGameplayTag& Key,
		const FVector& Value,
		bool bKeepValueOnPayloadReset = false);
	void SetObject(
		EMASkillPayloadScope Scope,
		const FGameplayTag& Key,
		UObject* Value,
		bool bKeepValueOnPayloadReset = false);

	template <typename StructType>
	void SetStruct(
		EMASkillPayloadScope Scope,
		const FGameplayTag& Key,
		const StructType& Value,
		bool bKeepValueOnPayloadReset = false)
	{
		if (FMASkillPayloadStore* Store = GetStore(Scope))
		{
			Store->SetStruct(Key, Value, bKeepValueOnPayloadReset);
		}
	}

private:
	FMASkillPayloadStore* GetStore(EMASkillPayloadScope Scope) const;

	FMASkillPayloadStore* SkillPayloads = nullptr;
	FMASkillPayloadStore* ModulePayloads = nullptr;
};
