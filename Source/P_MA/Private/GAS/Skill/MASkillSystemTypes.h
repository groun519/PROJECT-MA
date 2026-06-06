#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "MASkillSystemTypes.generated.h"

class UMASkillModuleInstance;
struct FGameplayAbilitySpec;

USTRUCT()
struct FMASkillSlotRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FGameplayTag SlotTag;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMASkillModuleInstance>> SourceModuleInstances;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillModuleInstance> AssembledModuleInstance = nullptr;

	UPROPERTY(Transient)
	FGameplayAbilitySpecHandle AbilityHandle;
};

USTRUCT(BlueprintType)
struct FMASkillSlotStack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill", meta=(Categories="Skill.Slot"))
	FGameplayTag SlotTag;
};

USTRUCT()
struct FMASkillReplicatedSlotRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FGameplayTag SlotTag;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMASkillModuleInstance>> ModuleInstances;
};

struct P_MA_API FMASkillSystemStatics
{
	static int32 ResolveSlotInputID(const FGameplayTag& SlotTag);
	static bool IsSkillSlotTag(const FGameplayTag& Tag);
	static FGameplayTag ResolveSlotTagFromAbilitySpec(const FGameplayAbilitySpec& Spec);

private:
	FMASkillSystemStatics() = delete;
};
