#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MASkillModuleInstance.generated.h"

class UMASkillDefinition;

UCLASS()
class P_MA_API UMASkillModuleInstance : public UObject
{
	GENERATED_BODY()

public:
	static UMASkillModuleInstance* Create(UObject* Outer, UMASkillDefinition* InDefinition);

	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UMASkillDefinition* GetDefinition() const { return Definition; }
	void SetDefinition(UMASkillDefinition* InDefinition) { Definition = InDefinition; }
	bool IsValid() const { return Definition != nullptr; }

private:
	UPROPERTY(Replicated)
	TObjectPtr<UMASkillDefinition> Definition;
};
