#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Module/MASkillModuleDataTypes.h"
#include "UObject/Object.h"
#include "MASkillModuleEditorObject.generated.h"

/** Transient object shown by the skill-module editor Details View. */
UCLASS(Transient)
class UMASkillModuleEditorObject : public UObject
{
	GENERATED_BODY()

public:
	void SetModule(const int32 InModuleId, FMASkillModuleData&& InModuleData)
	{
		ModuleId = InModuleId;
		ModuleData = MoveTemp(InModuleData);
	}

	void AssignModuleId(const int32 InModuleId) { ModuleId = InModuleId; }
	int32 GetModuleId() const { return ModuleId; }
	const FMASkillModuleData& GetModuleData() const { return ModuleData; }

private:
	UPROPERTY(VisibleAnywhere, Category="Module")
	int32 ModuleId = 0;

	UPROPERTY(EditAnywhere, Category="Module", meta=(ShowOnlyInnerProperties))
	FMASkillModuleData ModuleData;
};
