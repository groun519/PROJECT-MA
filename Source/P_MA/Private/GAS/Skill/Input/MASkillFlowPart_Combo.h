#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GAS/Skill/Input/MASkillFlowPart.h"
#include "TimerManager.h"
#include "MASkillFlowPart_Combo.generated.h"

class UAbilityTask_WaitInputPress;
class UAbilityTask_WaitInputRelease;

USTRUCT(BlueprintType)
struct FMASkillComboInputEvent
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Flow")
	FGameplayTag OpenTag;

	UPROPERTY(EditDefaultsOnly, Category="Flow")
	FName NextSectionName = NAME_None;
};

USTRUCT()
struct FMASkillComboInputLoopState
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitInputPress> InputPressTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitInputRelease> InputReleaseTask;

	FTimerHandle HoldTimerHandle;
};

USTRUCT()
struct FMASkillComboReservationState
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FName ReservedNextSection = NAME_None;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillFlowPart_Combo : public UMASkillFlowPart
{
	GENERATED_BODY()

public:
	virtual void StartFlow(UMASkillAbility* SkillAbility, FSkillRuntimeContext* InRuntimeContext) override;
	virtual void StopFlow() override;
	virtual void CollectRequiredEventTags(TSet<FGameplayTag>& OutTags) const override;
	virtual void HandleRuntimeEvent(const FGameplayEventData& Payload) override;

private:
	UFUNCTION()
	void HandleInputPressed(float TimeWaited);

	UFUNCTION()
	void HandleInputReleased(float TimeHeld);

	UFUNCTION()
	void HandleHoldTick();

	/** Input Loop **/
	void ArmInputPress();
	void ArmInputRelease();
	void StartHoldLoop();
	void StopInputLoop();
	void StopHoldLoop();

	void ClearReservedState();

	/** Section Reservation **/
	void CommitReservedNextSection();
	void ClearCurrentSectionLink();

	UPROPERTY(EditDefaultsOnly, Category="Flow")
	TArray<FMASkillComboInputEvent> ComboEvents;

	UPROPERTY(EditDefaultsOnly, Category="Flow")
	FGameplayTag CloseTag;

	UPROPERTY(EditDefaultsOnly, Category="Flow", meta=(ClampMin="0.01"))
	float HoldInterval = 0.01f;

	UPROPERTY(Transient)
	FMASkillComboInputLoopState InputLoopState;

	UPROPERTY(Transient)
	FMASkillComboReservationState ReservationState;
};
