#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "UObject/Object.h"
#include "MASkillModuleInstance.generated.h"

class UMASkillDefinition;
class UMASkillEventDispatcher;
class UMASkillRuntimeRegistry;
struct FMASkillEvent;

DECLARE_MULTICAST_DELEGATE(FMASkillModuleStateChangedSignature);

UCLASS()
class P_MA_API UMASkillModuleInstance : public UObject
{
	GENERATED_BODY()

public:
	UMASkillModuleInstance();

	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UMASkillDefinition* GetDefinition() const { return Definition; }
	void SetDefinition(UMASkillDefinition* InDefinition);
	bool IsValid() const { return Definition != nullptr; }
	bool IsActive() const { return bIsActive; }
	void SetActive(
		bool bInActive,
		const FGameplayTag& InInactiveReasonTag = FGameplayTag());
	const FGameplayTag& GetInactiveReasonTag() const { return InactiveReasonTag; }

	/** Module Stack **/
	bool IsStackEnabled() const;
	int32 GetStack() const { return Stack; }
	void SetStack(int32 NewStack);
	void AddStack(int32 Delta) { SetStack(Stack + Delta); }
	void ClearStack() { SetStack(0); }

	/** Module Cooldown **/
	bool IsCooldownActive() const;
	void RegisterCooldownEvents(
		UMASkillEventDispatcher& EventDispatcher,
		UMASkillModuleInstance* SkillScope);

	// Add a const getter when a const module instance needs read-only payload access.
	FMASkillPayloadStore& GetPayloadStore() { return PayloadStore; }
	void ResetPayloadStore() { PayloadStore.Reset(); }
	UMASkillRuntimeRegistry* GetRuntimeRegistry() const { return RuntimeRegistry; }

	FMASkillModuleStateChangedSignature OnStateChanged;

private:
	UFUNCTION()
	void OnRep_Definition();
	UFUNCTION()
	void OnRep_Stack();
	void InitializePayloadStore();
	void RefreshStackPayload();

	UPROPERTY(ReplicatedUsing=OnRep_Definition)
	TObjectPtr<UMASkillDefinition> Definition;

	UPROPERTY(Transient)
	bool bIsActive = true;

	UPROPERTY(Transient)
	FGameplayTag InactiveReasonTag;

	// TODO(Addon): If addon-specific runtime state grows, split it into a module runtime data layer
	// instead of continuing to add optional feature fields directly to UMASkillModuleInstance.
	UPROPERTY(ReplicatedUsing=OnRep_Stack)
	int32 Stack = 0;

	UPROPERTY(Transient)
	FMASkillPayloadStore PayloadStore;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillRuntimeRegistry> RuntimeRegistry;

	/** Module Cooldown **/
	UFUNCTION()
	void RefreshModuleCooldownState();
	void HandleCooldownEvent(
		const FMASkillEvent& Event,
		TWeakObjectPtr<UMASkillModuleInstance> SkillScope);
	float GetCurrentServerTimeSeconds() const;

	UPROPERTY(ReplicatedUsing=RefreshModuleCooldownState)
	float ModuleCooldownEndTimeSeconds = 0.f;

	FTimerHandle ModuleCooldownTimerHandle;

	friend struct FMASkillAssembler;
};

