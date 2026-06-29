#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Step/MASkillStep_Timed.h"
#include "MASkillStep_Cast.generated.h"

UENUM(BlueprintType)
enum class EMASkillCastMontageMode : uint8
{
	CustomMontage,
	BlendInNextMontage
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillStep_Cast : public UMASkillStep_Timed
{
	GENERATED_BODY()

public:
	UMASkillStep_Cast();
	void Configure(float InCastDuration, EMASkillCastMontageMode InMontageMode, UAnimMontage* InCustomMontage);

private:
	virtual float GetStepDuration() const override { return CastDuration; }
	virtual UAnimMontage* ResolveStepMontage() const override;
	virtual void OnTimedStepStarted(UMASkillAbility*, EMASkillStepStartMode StartMode) override;
	virtual void OnTimedStepStopped() override;
	void ApplyInputBlockTag();
	void RemoveInputBlockTag();

	UPROPERTY(EditDefaultsOnly, Category="Cast")
	EMASkillCastMontageMode MontageMode = EMASkillCastMontageMode::CustomMontage;

	UPROPERTY(EditDefaultsOnly, Category="Cast", meta=(ClampMin="0.0"))
	float CastDuration = 0.f;

	UPROPERTY(Transient)
	bool bAppliedInputBlockTag = false;
};
