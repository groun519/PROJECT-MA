#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GAS/Skill/Payload/MASkillPayloadAccessor.h"
#include "MASkillEventTypes.generated.h"

class UMASkillModuleInstance;
class UMASkillRuntimeRegistry;

UENUM(BlueprintType)
enum class EMASkillEventBindingScope : uint8
{
	Module,
	Skill,
	Global
};

USTRUCT()
struct P_MA_API FMASkillScopes
{
	GENERATED_BODY()

	FMASkillScopes() = default;
	FMASkillScopes(UMASkillModuleInstance* InModule, UMASkillModuleInstance* InSkill)
		: Module(InModule), Skill(InSkill) {}

	UPROPERTY(Transient)
	TObjectPtr<UMASkillModuleInstance> Module = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillModuleInstance> Skill = nullptr;

	FMASkillPayloadAccessor GetPayloadAccess(const FMASkillPayloadStore* EventPayloads = nullptr) const;
	FMASkillPayloadAccessor GetPayloadAccess(const FMASkillPayloadStore& EventPayloads) const { return GetPayloadAccess(&EventPayloads); }
	UMASkillRuntimeRegistry& GetRuntimeRegistry() const;
};

USTRUCT()
struct P_MA_API FMASkillEvent
{
	GENERATED_BODY()

	FMASkillEvent() = default;
	FMASkillEvent(FGameplayTag InTag, const FMASkillScopes& InSourceScopes = FMASkillScopes())
		: Tag(InTag), SourceScopes(InSourceScopes) {}

	UPROPERTY(Transient)
	FGameplayTag Tag;

	UPROPERTY(Transient)
	FMASkillPayloadStore Payloads;

	UPROPERTY(Transient)
	FMASkillScopes SourceScopes;

	void SetMagnitude(float Magnitude);
	float GetMagnitude() const;
	void SetTargetData(const FGameplayAbilityTargetDataHandle& TargetData);
	const FGameplayAbilityTargetDataHandle* GetTargetData() const;
	FMASkillPayloadAccessor GetPayloadAccess(const FMASkillScopes& BindingScopes) const
	{
		return BindingScopes.GetPayloadAccess(Payloads);
	}
};
