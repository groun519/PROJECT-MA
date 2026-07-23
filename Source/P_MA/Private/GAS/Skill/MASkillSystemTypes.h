#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "MASkillSystemTypes.generated.h"

class UMASkillModule;
class UMASkillModuleInstance;
struct FGameplayAbilitySpec;

/** One root module and its scope-free submodules, composed as a single module contribution. */
USTRUCT()
struct FMASkillModuleGroup
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UMASkillModule> RootModule = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<UMASkillModule>> SubModules;
};

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill", meta=(Categories="Skill.Slot.Active"))
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
	static bool IsActiveSkillSlotTag(const FGameplayTag& Tag);
	static bool IsPassiveSkillSlotTag(const FGameplayTag& Tag);
	static FGameplayTag GetPassiveSlotTag();
	static FGameplayTag ResolveCooldownTagFromSlotTag(const FGameplayTag& SlotTag);
	static FGameplayTag ResolveSlotTagFromAbilitySpec(const FGameplayAbilitySpec& Spec);

private:
	FMASkillSystemStatics() = delete;
};
