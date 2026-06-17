#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "MASkillWarningTextData.generated.h"

USTRUCT(BlueprintType)
struct P_MA_API FMASkillWarningTextDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Warning", meta=(Categories="Module.Assembly.Exclusive"))
	FGameplayTag ReasonTag;

	UPROPERTY(EditAnywhere, Category="Warning", meta=(MultiLine=true))
	FText WarningText;

	UPROPERTY(EditAnywhere, Category="Warning")
	FLinearColor TagBackgroundColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.f);

	UPROPERTY(EditAnywhere, Category="Warning")
	FLinearColor TagTextColor = FLinearColor::White;
};
