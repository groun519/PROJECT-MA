#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "MASkillModuleInstance.generated.h"

class UMASkillDefinition;
class UMASkillRuntimeRegistry;

UENUM(BlueprintType)
enum class EMASkillModuleActivationState : uint8
{
	Active,
	Inactive
};

UCLASS()
class P_MA_API UMASkillModuleInstance : public UObject
{
	GENERATED_BODY()

public:
	static UMASkillModuleInstance* Create(UObject* Outer, UMASkillDefinition* InDefinition);

	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UMASkillDefinition* GetDefinition() const { return Definition; }
	void SetDefinition(UMASkillDefinition* InDefinition);
	bool IsValid() const { return Definition != nullptr; }
	bool IsActive() const { return ActivationState == EMASkillModuleActivationState::Active; }
	void SetActivationState(EMASkillModuleActivationState InActivationState, const FGameplayTag& InInactiveReasonTag = FGameplayTag())
	{
		ActivationState = InActivationState;
		InactiveReasonTag = IsActive() ? FGameplayTag() : InInactiveReasonTag;
	}
	const FGameplayTag& GetInactiveReasonTag() const { return InactiveReasonTag; }
	// Add a const getter when a const module instance needs read-only payload access.
	FMASkillPayloadStore& GetPayloadStore() { return PayloadStore; }
	void ResetPayloadStore() { PayloadStore.Reset(); }
	UMASkillRuntimeRegistry* GetRuntimeRegistry() const { return RuntimeRegistry; }

private:
	UFUNCTION()
	void OnRep_Definition();
	void InitializePayloadStore();

	UPROPERTY(ReplicatedUsing=OnRep_Definition)
	TObjectPtr<UMASkillDefinition> Definition;

	UPROPERTY(Transient)
	EMASkillModuleActivationState ActivationState = EMASkillModuleActivationState::Active;

	UPROPERTY(Transient)
	FGameplayTag InactiveReasonTag;

	UPROPERTY(Transient)
	FMASkillPayloadStore PayloadStore;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillRuntimeRegistry> RuntimeRegistry;

	friend struct FMASkillAssembler;
};
