#pragma once

#include "AttributeSet.h"
#include "CoreMinimal.h"
#include "GAS/Skill/Event/Source/MASkillEventSource.h"
#include "MASkillEventSource_AttributeChanged.generated.h"

struct FOnAttributeChangeData;

UENUM(BlueprintType)
enum class EMAAttributeChangeType : uint8
{
	Changed,
	Increased,
	Decreased
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillEventSource_AttributeChanged : public UMASkillEventSource
{
	GENERATED_BODY()

public:
	UMASkillEventSource_AttributeChanged();

protected:
	virtual void StartSource() override;
	virtual void StopSource() override;

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void PostLoad() override;

private:
	virtual bool RequiresRuntimeInstance() const override { return true; }
	void HandleAttributeChanged(const FOnAttributeChangeData& ChangeData);
	void RefreshEmittedTag();

	UPROPERTY(EditDefaultsOnly, Category="Event")
	FGameplayAttribute ChangedAttribute;

	UPROPERTY(EditDefaultsOnly, Category="Event")
	EMAAttributeChangeType ChangeType = EMAAttributeChangeType::Changed;

	FDelegateHandle AttributeChangedHandle;
};
