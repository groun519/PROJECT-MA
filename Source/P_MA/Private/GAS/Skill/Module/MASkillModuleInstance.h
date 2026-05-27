#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "MASkillModuleInstance.generated.h"

class UMASkillDefinition;
struct FGameplayEventData;

DECLARE_MULTICAST_DELEGATE_TwoParams(FMASkillScopedEventSignature, const FGameplayTag&, const FGameplayEventData&);

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
	FMASkillScopedEventSignature& OnScopedEvent() { return ScopedEventDelegate; }
	void BroadcastScopedEvent(const FGameplayTag& SourceEventTag, const FGameplayEventData& EventData);
	// Add a const getter when a const module instance needs read-only payload access.
	FMASkillPayloadStore& GetPayloadStore() { return PayloadStore; }
	void ResetPayloadStore() { PayloadStore.Reset(); }

private:
	UPROPERTY(Replicated)
	TObjectPtr<UMASkillDefinition> Definition;

	UPROPERTY(Transient)
	FMASkillPayloadStore PayloadStore;

	FMASkillScopedEventSignature ScopedEventDelegate;
};
