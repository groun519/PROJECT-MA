#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Addon/MASkillModuleAddon.h"
#include "MASkillModuleItemAddon.generated.h"

class AActor;
class UMASkillModule;
struct FGameplayTag;

/** Owns the direct-use behavior of an Item module. */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Item")
class P_MA_API UMASkillModuleItemAddon : public UMASkillModuleAddon
{
	GENERATED_BODY()

public:
	UMASkillModuleItemAddon() { SupportedModuleTypes = EMASkillModuleType::Item; }

	static const FGameplayTag& GetUseEventTag();
	void Use(AActor& Owner, const UMASkillModule& Module) const;
	const FMAStaticMeshVisualData& GetWorldMesh() const { return WorldMesh; }

private:
	/** Optional unique mesh used when this item is represented in the world. */
	UPROPERTY(EditDefaultsOnly, Category="World")
	FMAStaticMeshVisualData WorldMesh;
};
