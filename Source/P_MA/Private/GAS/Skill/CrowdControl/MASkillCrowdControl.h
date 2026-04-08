#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GAS/Skill/CrowdControl/MASkillCrowdControlTypes.h"
#include "MASkillCrowdControl.generated.h"

class UMASkillAbility;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillCrowdControl : public UObject
{
	GENERATED_BODY()

public:
	bool BuildResolvedEffect(UMASkillAbility& SkillAbility, TArray<FResolvedCrowdControlEffect>& OutEffects) const;

protected:
	virtual bool ResolveSpecData(
		FGameplayTag& OutCrowdControlTag,
		float& OutMagnitude,
		float& OutDuration,
		EMASkillCrowdControlSourceType& OutSourceType) const
		PURE_VIRTUAL(UMASkillCrowdControl::ResolveSpecData, return false;);

	virtual void ApplyCustomPayload(FGameplayEffectSpecHandle& SpecHandle) const {}
};

UCLASS(Abstract, BlueprintType)
class P_MA_API UMASkillCrowdControlStateBase : public UMASkillCrowdControl
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category="CrowdControl", meta=(ClampMin="0.0"))
	float Duration = 0.f;

	virtual FGameplayTag GetCrowdControlTag() const
		PURE_VIRTUAL(UMASkillCrowdControlStateBase::GetCrowdControlTag, return FGameplayTag(););

	virtual bool ResolveSpecData(
		FGameplayTag& OutCrowdControlTag,
		float& OutMagnitude,
		float& OutDuration,
		EMASkillCrowdControlSourceType& OutSourceType) const override;
};

UCLASS(Abstract, BlueprintType)
class P_MA_API UMASkillCrowdControlImpulseBase : public UMASkillCrowdControl
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category="CrowdControl", meta=(ClampMin="0.0"))
	float Magnitude = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl", meta=(ClampMin="0.0"))
	float Duration = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl")
	EMASkillCrowdControlSourceType SourceType = EMASkillCrowdControlSourceType::Instigator;

	virtual FGameplayTag GetCrowdControlTag() const
		PURE_VIRTUAL(UMASkillCrowdControlImpulseBase::GetCrowdControlTag, return FGameplayTag(););

	virtual bool ResolveSpecData(
		FGameplayTag& OutCrowdControlTag,
		float& OutMagnitude,
		float& OutDuration,
		EMASkillCrowdControlSourceType& OutSourceType) const override;
};

UCLASS(BlueprintType, DisplayName="CC Stun")
class P_MA_API UMASkillCrowdControlStun : public UMASkillCrowdControlStateBase
{
	GENERATED_BODY()

protected:
	virtual FGameplayTag GetCrowdControlTag() const override;
};

UCLASS(BlueprintType, DisplayName="CC Root")
class P_MA_API UMASkillCrowdControlRoot : public UMASkillCrowdControlStateBase
{
	GENERATED_BODY()

protected:
	virtual FGameplayTag GetCrowdControlTag() const override;
};

UCLASS(BlueprintType, DisplayName="CC Knockback")
class P_MA_API UMASkillCrowdControlKnockback : public UMASkillCrowdControlImpulseBase
{
	GENERATED_BODY()

protected:
	virtual FGameplayTag GetCrowdControlTag() const override;
};

UCLASS(BlueprintType, DisplayName="CC Grab")
class P_MA_API UMASkillCrowdControlGrab : public UMASkillCrowdControlImpulseBase
{
	GENERATED_BODY()

protected:
	virtual FGameplayTag GetCrowdControlTag() const override;
};

UCLASS(BlueprintType, DisplayName="CC Stagger")
class P_MA_API UMASkillCrowdControlStagger : public UMASkillCrowdControlImpulseBase
{
	GENERATED_BODY()

protected:
	virtual FGameplayTag GetCrowdControlTag() const override;
};

UCLASS(BlueprintType, DisplayName="CC Airborne")
class P_MA_API UMASkillCrowdControlAirborne : public UMASkillCrowdControl
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category="CrowdControl", meta=(ClampMin="0.0"))
	float Magnitude = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl", meta=(ClampMin="0.0"))
	float Duration = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl", meta=(ClampMin="0.0"))
	float RiseTime = 0.f;

	virtual bool ResolveSpecData(
		FGameplayTag& OutCrowdControlTag,
		float& OutMagnitude,
		float& OutDuration,
		EMASkillCrowdControlSourceType& OutSourceType) const override;

	virtual void ApplyCustomPayload(FGameplayEffectSpecHandle& SpecHandle) const override;
};
