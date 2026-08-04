#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "MASkillWarningTextData.generated.h"

UENUM(BlueprintType)
enum class EMASkillTooltipTextType : uint8
{
	Normal,
	Warning
};

USTRUCT(BlueprintType)
struct P_MA_API FMASkillWarningTextDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Warning")
	EMASkillTooltipTextType TextType = EMASkillTooltipTextType::Warning;

	UPROPERTY(EditAnywhere, Category="Warning", meta=(Categories="Module"))
	FGameplayTag ReasonTag;

	UPROPERTY(EditAnywhere, Category="Warning", meta=(MultiLine=true))
	FText WarningText;

	UPROPERTY(EditAnywhere, Category="Warning")
	FLinearColor TagBackgroundColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.f);

	UPROPERTY(EditAnywhere, Category="Warning")
	FLinearColor TagTextColor = FLinearColor::White;
};
