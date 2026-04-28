#pragma once

#include "AttributeSet.h"
#include "CoreMinimal.h"
#include "GAS/Skill/Event/Publish/MASkillEventSource.h"
#include "MASkillEventSource_AttributeChanged.generated.h"

struct FOnAttributeChangeData;

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillEventSource_AttributeChanged : public UMASkillEventSource
{
	GENERATED_BODY()

public:
	UMASkillEventSource_AttributeChanged();

	virtual void StartSource(UMASkillAbility* SkillAbility) override;
	virtual void StopSource() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void PostLoad() override;

private:
	void HandleAttributeChanged(const FOnAttributeChangeData& ChangeData);
	void RefreshEmittedTag();

	UPROPERTY(EditDefaultsOnly, Category="Event")
	FGameplayAttribute ChangedAttribute;

	FDelegateHandle AttributeChangedHandle;
};
