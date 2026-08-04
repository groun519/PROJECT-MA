#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "MAElementData.generated.h"

class UNiagaraSystem;

USTRUCT(BlueprintType)
struct P_MA_API FMAElementDataRow : public FTableRowBase
{
	GENERATED_BODY()

	static const FMAElementDataRow* FindByTag(const FGameplayTag& SourceTag, const TCHAR* ContextString);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Element")
	FLinearColor ElementColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Element")
	TObjectPtr<UNiagaraSystem> MainVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Element")
	TObjectPtr<UNiagaraSystem> TrailVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Element", meta=(Categories="GameplayCue"))
	FGameplayTag HitGameplayCueTag;
};
