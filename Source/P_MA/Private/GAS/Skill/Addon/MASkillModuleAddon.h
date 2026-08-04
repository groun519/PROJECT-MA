#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Definition/Assembly/MASkillAddonAssembler.h"
#include "GAS/Skill/Module/MASkillModuleTypes.h"
#include "UObject/Object.h"
#include "MASkillModuleAddon.generated.h"

struct FMASkillModuleAddonRuntimeData;
struct FMASkillPayloadStore;
class UMASkillEventDispatcher;
class UMASkillModuleInstance;

/**
 * Definition-only extension point for optional module features.
 * Runtime state stays on the module instance as typed replicated data.
 */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillModuleAddon : public UObject
{
	GENERATED_BODY()

public:
	bool SupportsModuleType(const EMASkillModuleType ModuleType) const
	{
		return EnumHasAnyFlags(SupportedModuleTypes, ModuleType);
	}

#if WITH_EDITOR
	/** Rebuilds serialized data derived from authored addon fields. */
	virtual void BuildGeneratedData() {}
#endif
	/** Merges this source addon into the accumulated result addon. */
	virtual UMASkillModuleAddon* AssembleInto(
		UObject& ResultOuter,
		UMASkillModuleAddon* ResultAddon,
		EMASkillAddonAssemblyStage Stage,
		const FMASkillScopes& SourceScopes) const { return nullptr; }
	/** Completes deferred assembly work; false removes an empty result addon. */
	virtual bool Finalize(EMASkillAddonAssemblyStage Stage) { return true; }
	virtual void InitializeRuntimeData(FMASkillModuleAddonRuntimeData& RuntimeData) const {}
	virtual void ApplyPayloadMirror(const FMASkillModuleAddonRuntimeData& RuntimeData, FMASkillPayloadStore& PayloadStore) const {}
	virtual void BindModule(UMASkillModuleInstance& ModuleInstance) const {}
	virtual void UnbindModule(UMASkillModuleInstance& ModuleInstance) const {}
	virtual void RegisterEventSubscriptions(
		UMASkillEventDispatcher& EventDispatcher,
		UMASkillModuleInstance& ModuleInstance,
		UMASkillModuleInstance& SkillScope) const {}
	virtual bool TryResolveSocketText(const FMASkillModuleAddonRuntimeData& RuntimeData, FText& OutText) const { return false; }

protected:
	/** Module types that may author this addon. Concrete addons declare this in their constructor. */
	EMASkillModuleType SupportedModuleTypes = EMASkillModuleType::None;
};
